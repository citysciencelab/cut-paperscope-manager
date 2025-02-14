/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#include "PSCalibrate.h"

	// Qt
	#include <QApplication>
	#include <QMediaDevices>
	#include <QCameraDevice>

	// App
	#include "../../global/Settings.h"



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONSTRUCTOR
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	PSCalibrate::PSCalibrate(QObject *parent)
		: QObject(parent),
		  frameCount(10)
	{
		
	}


	PSCalibrate::~PSCalibrate() {

	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PROCESSING
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSCalibrate::init() {

		tickCount = cv::getTickCount();
		timeCount = 0;
	}


	bool PSCalibrate::update(cv::Mat *mTracking, cv::Mat *mRender, PSTrackingMode trackingMode) {
		
		if(trackingMode != PSTrackingMode::Calibrate) { return false; }

		// calculate milliseconds per frame
		double time = (cv::getTickCount() - tickCount) / cv::getTickFrequency();
		tickCount = cv::getTickCount();

		// draw detected corners count
		std::string count = std::to_string(detectedCorners.size()) + "/" + std::to_string(frameCount);
		cv::putText(*mRender, count, cv::Point(30, 40), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255,255,0), 4, cv::LINE_AA);
		
		// update timer
		timeCount += time * 1000;
		if(timeCount < 7000) { 
			// draw countdown
			int countdown = 7 - timeCount / 1000;
			cv::putText(*mRender, std::to_string(countdown), cv::Point(30, 80), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255,255,0), 4, cv::LINE_AA);
			return false;; 
		}
		timeCount = 0;

		// find checkerboard corners
		std::vector<cv::Point2f> corners;
		bool success = cv::findChessboardCorners(*mTracking, cv::Size(9,6), corners);
		if(!success) { return false; }
		cv::drawChessboardCorners(*mRender, cv::Size(9,6), corners, success);

		// save corners with subpixel accuracy
		cv::Mat gray;
		cv::cvtColor(*mTracking, gray, cv::COLOR_BGR2GRAY);
		cv::cornerSubPix(gray, corners, cv::Size(11,11), cv::Size(-1,-1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1));
		detectedCorners.push_back(corners);

		if(frameCount == (int)detectedCorners.size()) {
			createCalibration();
			return false;
		}
		
		return true;
	}


	void PSCalibrate::close() {
		
		detectedCorners.clear();
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CALIBRATION
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	/**
	 * Calculate the camera calibration matrix and distortion coefficients based on the detected corners.
	 */

	void PSCalibrate::createCalibration() {
		
		std::vector<cv::Point3f> obj;
		std::vector<std::vector<cv::Point3f>> objPoints;
		cv::Mat cameraMatrix, distCoeffs;
		std::vector<cv::Mat> rvecs, tvecs;
		
		// create 3d object points for the chessboard
		for(int i=0; i<6; i++) {
			for(int j=0; j<9; j++) {
				obj.push_back(cv::Point3f(j, i, 0));
			}
		}
		
		// create object points for each detected corner
		for(size_t i=0; i < detectedCorners.size(); i++) {
			objPoints.push_back(obj);
		}
		
		cv::calibrateCamera(objPoints, detectedCorners, cv::Size(9,6), cameraMatrix, distCoeffs, rvecs, tvecs);
		saveCalibration(cameraMatrix, distCoeffs);
	}


	void PSCalibrate::saveCalibration(cv::Mat cameraMatrix, cv::Mat distCoeffs) {

		QString selectedCamera = Settings::instance()->getString("cameraDevice");
		selectedCamera.replace(" ", "_");

		Settings::instance()->saveMat("cameraMatrix_"+selectedCamera, cameraMatrix);
		Settings::instance()->saveMat("distCoeffs_"+selectedCamera, distCoeffs);
		Settings::instance()->saveString("calibration_mode", "auto");

		emit calibrationCompleted();
	}


	
