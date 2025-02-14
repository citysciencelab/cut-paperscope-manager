/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#include "PSCapture.h"

	// Qt
	#include <QDebug>
	#include <QMediaDevices>
	#include <QCameraDevice>
	#include <QRegExp>

	// OpenCV
	#include <opencv2/objdetect/aruco_detector.hpp>
	#include <opencv2/aruco.hpp>

	// App
	#include "../../global/Settings.h"



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONSTRUCTOR
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	PSCapture::PSCapture(QObject *parent)
		: QObject(parent),
		  capture(nullptr),
		  matTracking(nullptr),
		  matRender(nullptr),
		  renderMode(RenderMode::Camera)
	{

		// init properties
		capture = new cv::VideoCapture();
		smoothingFactor = Settings::instance()->getFloat("smoothing", 0.8f);
		scalingFactor = Settings::instance()->getFloat("scaling", 1.0f);
		calibrationMode = Settings::instance()->getString("calibration_mode", "auto");
        manualImagePoints = Settings::instance()->getPoints("calibration_points");

		// init member
		updatePlaneSize();
	}


	PSCapture::~PSCapture() {

		delete capture;
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PROCESSING
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSCapture::init() {

		openCamera();
		setCameraProps();
		loadCameraCalibration();
	}


	void PSCapture::update(cv::Mat *mTracking, cv::Mat *mRender, PSTrackingMode trackingMode) {

		matTracking = mTracking;
		matRender = mRender;

		if(!capture->read(*matTracking)) { return; }
		*matRender = matTracking->clone();
		
		if(trackingMode != PSTrackingMode::Calibrate) {
			processImage();
			findArucoMarker();
			drawArucoMarker();
		}

		if(trackingMode != PSTrackingMode::None && trackingMode != PSTrackingMode::Calibrate) {
			estimateMarkerPose();
			get2DPlane();
		}
	}


	void PSCapture::close() {

		capture->release();
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	IMAGE PROCESSING
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSCapture::processImage() {

	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	ARUCO MARKER
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	/**
	 * Corner refinement does not work well with compressed camera frames (only raw sensor data).
	 */

	void PSCapture::findArucoMarker(int id) {

		cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_ARUCO_ORIGINAL);

		// aruco config		
		cv::aruco::DetectorParameters detectorParams = cv::aruco::DetectorParameters();
		detectorParams.useAruco3Detection = true;

		// detect marker
		std::vector<int> markers;
		std::vector<std::vector<cv::Point2f>> corners;
		cv::aruco::ArucoDetector detector(dictionary, detectorParams);
		detector.detectMarkers(*matTracking, corners, markers);

		// get only the selected marker
		markerIds.clear();
		markerCorners.clear();
		for(size_t i = 0; i < markers.size(); ++i) {
			if (markers[i] == id) {
				markerIds.push_back(markers[i]);
				markerCorners.push_back(corners[i]);
				break;
			}
		}
	}


	void PSCapture::drawArucoMarker() {

		if(markerIds.size() == 0) { return; }

		cv::aruco::drawDetectedMarkers(*matRender, markerCorners, markerIds);
	}


	void PSCapture::estimateMarkerPose() {

		if(markerIds.size() == 0) { return; }

		rvecs.clear();
		tvecs.clear();

		cv::aruco::estimatePoseSingleMarkers(markerCorners, markerSize, cameraMatrix, distCoeffs, rvecs, tvecs);
		if(rvecs.size() == 0) { return; }
		 
		// draw axis
		if(renderMode == RenderMode::Camera) {
			for(size_t i = 0; i < rvecs.size(); i++) {
				cv::drawFrameAxes(*matRender, cameraMatrix, distCoeffs, rvecs[i], tvecs[i], 0.02f);
			}
		}
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	2D PLANE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSCapture::get2DPlane() {

		// skip if no marker found
		if(markerIds.size() == 0 && calibrationMode == "auto") { 
			currentImagePoints.clear();
			*matTracking = cv::Mat();
			return; 
		}
		
		// calculate 3d points of plane from marker size
		std::vector<cv::Point3f> corners;
		corners.push_back(cv::Point3f(markerSize * -0.5, markerSize * 0.5, 0));								// top left
		corners.push_back(cv::Point3f(markerSize * -0.5 + planeWidth, markerSize * 0.5, 0)); 				// top right
		corners.push_back(cv::Point3f(markerSize * -0.5 + planeWidth, markerSize * 0.5 - planeHeight, 0));	// bottom right
		corners.push_back(cv::Point3f(markerSize * -0.5, markerSize * 0.5 - planeHeight, 0));				// bottom left

		// project 3d points to 2d camera plane
		std::vector<cv::Point2f> cornerPoints;
		if(calibrationMode == "auto") {
			cv::projectPoints(corners, rvecs[0], tvecs[0], cameraMatrix, distCoeffs, cornerPoints);
			applySmoothing(cornerPoints);
		}
		else {
			currentImagePoints = manualImagePoints;
		}

		// create plane mat from width and height
		float w = 960;
		float h = planeHeight * 960/planeWidth;

		// 2d plane points
		std::vector<cv::Point2f> plane;
		plane.push_back(cv::Point2f(0, 0));
		plane.push_back(cv::Point2f(w, 0));
		plane.push_back(cv::Point2f(w, h));
		plane.push_back(cv::Point2f(0, h));

		// warp perspective
		if(currentImagePoints.size() > 0) {
			cv::Mat homography = cv::findHomography(currentImagePoints, plane);
			cv::warpPerspective(*matTracking, *matTracking, homography, cv::Size(w,h));
		}

		// replace aruoco marker with white shape (add 20px border)
		w = markerSize * 960/planeWidth + 40.0;
		h = markerSize * h/planeHeight + 40.0;
		cv::rectangle(*matTracking, cv::Point(0,0), cv::Point(w,h), cv::Scalar(255,255,255), -1);

		if(renderMode == RenderMode::PaperScope) {
			*matRender = matTracking->clone();
		}
	}


	void PSCapture::updatePlaneSize() {

		float defaultRatio = 0.300f / 0.210f;
		float ratio = defaultRatio;
		
		// update ratio from project
		if(project.contains("ratio")) {
			ratio = project["ratio"].toDouble();
		}

		// marker props in meters
		markerPadding = 0.013f;
		markerSize = 0.030f;

		// plane size in meters
		if(ratio >= 0.300f / 0.210f) {
			planeWidth = 0.300f - markerPadding * 2;
            planeHeight = (0.300f/ratio) - (markerPadding * 2);
		}
		else {
            planeWidth = (0.210f * ratio) - (markerPadding * 2);
			planeHeight = 0.210f - markerPadding * 2;
		}

		planeWidth *= scalingFactor;
		planeHeight *= scalingFactor;
	}

 

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SMOOTHING
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSCapture::applySmoothing(std::vector<cv::Point2f> points) {

		if(currentImagePoints.size() == 0) {
			currentImagePoints = points;
			return;
		}

		// simple LERP
		for(size_t i = 0; i < points.size(); i++) {
			currentImagePoints[i] = currentImagePoints[i] * smoothingFactor + points[i] * (1.0 - smoothingFactor);
		}
	}	



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CAMERA
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSCapture::openCamera() {

		QString selectedCamera = Settings::instance()->getString("cameraDevice");
		int index = 0;

		// get selected/saved camera
		if(!selectedCamera.isEmpty()) {
			QList<QCameraDevice> availableCameras = QMediaDevices::videoInputs();
			for(int i = 0; i < availableCameras.size(); i++) {
				if(availableCameras[i].description() == selectedCamera) { 
					index = i;
					break; 
				}
			}
		}

		// open camera
		#if defined(Q_OS_WIN)
			capture->open(index, cv::CAP_DSHOW);
			// capture->open(index + 1 + cv::CAP_DSHOW);
		#elif defined(Q_OS_MAC)
			capture->open(index);
		#endif
	}


	/**
	 * Set camera properties. Must be called after opening the camera.
	 */

	void PSCapture::setCameraProps() {

		QString selectedFormat = Settings::instance()->getString("cameraFormat");

		int width = 1280;
		int height = 720;
		int fps = 60;
		std::string pixelFormat = "NV12";

		// parse saved format
		QRegExp rx("(\\d+)x(\\d+) - (\\d+)fps \\((\\w+)\\)");
		if(rx.indexIn(selectedFormat) != -1) {
			width = rx.cap(1).toInt();
			height = rx.cap(2).toInt();
			fps = rx.cap(3).toInt();
			pixelFormat = rx.cap(4).toStdString();
		}

		// set format
		capture->set(cv::CAP_PROP_FRAME_WIDTH, width);
		capture->set(cv::CAP_PROP_FRAME_HEIGHT, height);
		capture->set(cv::CAP_PROP_FPS, fps);
		capture->set(cv::CAP_PROP_AUTO_EXPOSURE, 0.75);

		#if defined(Q_OS_WIN)
			capture->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G'));
		#elif defined(Q_OS_MAC)
			capture->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc(pixelFormat[0], pixelFormat[1], pixelFormat[2], pixelFormat[3]));
		#endif
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CALIBRATION
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSCapture::loadCameraCalibration() {
		
		QString selectedCamera = Settings::instance()->getString("cameraDevice");
		selectedCamera.replace(" ", "_");

		// make default camera calibration
		cameraMatrix = Settings::instance()->getMat("cameraMatrix_"+selectedCamera,cv::Mat::eye(3, 3, CV_64F));
		distCoeffs = Settings::instance()->getMat("distCoeffs_"+selectedCamera,cv::Mat::zeros(5, 1, CV_64F));
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SETTINGS
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSCapture::onSettingsUpdated(QString key, QVariant value) {

		if(key == "cameraMatrix" || key == "distCoeffs") { 
			loadCameraCalibration();
		}
		else if(key == "renderMode") {
			renderMode = (RenderMode) value.toInt();
		}
		else if(key == "project") {
			project = value.toJsonObject();
			updatePlaneSize();
		}
		else if(key == "smoothing") {
			smoothingFactor = value.toFloat();
		}
		else if(key == "scaling") {
			scalingFactor = value.toFloat();
			updatePlaneSize();
		}
		else if(key == "calibration_mode") {
			calibrationMode = value.toString();
		}
		else if(key == "calibration_points") {

			QList<QVariant> list = value.toList();
			manualImagePoints.clear();

			for(int i = 0; i < list.size(); i++) {
				QPointF point = list[i].toPointF();
				manualImagePoints.push_back(cv::Point2f(point.x(), point.y()));
			}
		}
	}


