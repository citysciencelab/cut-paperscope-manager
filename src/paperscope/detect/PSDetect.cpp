/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#include "PSDetect.h"

	// Qt
	#include <QCoreApplication>
	#include <QDebug>
	#include <QRandomGenerator>
	#include <QDir>
	#include <QStandardPaths>

	// OpenCV
	#include <opencv2/xphoto/white_balance.hpp>
	#include <opencv2/ximgproc.hpp>
	
	// Tensorflow
	#include "tensorflow/lite/model.h"
	#include "tensorflow/lite/tools/gen_op_registration.h"

	// App
	#include "../../global/Settings.h"



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONSTRUCTOR
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	PSDetect::PSDetect(QObject *parent)
		: QObject(parent),
		  matTracking(nullptr),
		  matProcessing(nullptr),
		  matThreshold(nullptr),
		  matRender(nullptr),
		  matStreets(nullptr),
		  model(nullptr),
		  resolver(nullptr),
	 	  interpreter(nullptr)
	{

		// init properties
		matProcessing = new cv::Mat();
		matThreshold = new cv::Mat();
		matStreets = new cv::Mat();
		thresholdDark = Settings::instance()->getInt("threshold_dark",50);
		thresholdLight = Settings::instance()->getInt("threshold_light",180);
		thresholdRed = Settings::instance()->getInt("threshold_red",150);
		renderMode = RenderMode::Camera,
		viewMode = PSViewMode::Threshold;

		// init member
		initAI();
	}


	PSDetect::~PSDetect() {

		delete matProcessing;
		delete matThreshold;
		delete matStreets;
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PROCESSING
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSDetect::init() {

	}


	void PSDetect::update(cv::Mat *mTracking, cv::Mat *mRender, PSTrackingMode trackingMode) {

		matTracking = mTracking;
		matRender = mRender;

		// skip loop
		if(trackingMode == PSTrackingMode::None || matTracking->empty()) {
			
			if(renderMode == RenderMode::PaperScope) {
				*matRender = cv::Mat::zeros(cv::Size(matRender->cols, matRender->rows), CV_8UC3);
			}
			return;
		}

		imageProcessing();
		applyThreshold();

		if(trackingMode == PSTrackingMode::Tracking) {

			findContours();
			findStreets();
			drawCandidates();

			// prepare tracking mat for PSDescribe
			cv::erode(*matThreshold, *matThreshold, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));
			cv::cvtColor(*matThreshold, *matThreshold, cv::COLOR_GRAY2BGR);
			cv::bitwise_and(*matTracking, *matThreshold, *matTracking);
		}
	}


	void PSDetect::close() {

	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	IMAGE PROCESSING
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	/**
	 * Optimize contrast/difference between background and foreground.
	 */

	void PSDetect::imageProcessing() {

		// white balance
		//whiteBalance(*matTracking);

		// convert rgb to hsv
		cv::cvtColor(*matTracking, *matProcessing, cv::COLOR_BGR2HSV);

		// split hsv channels
		std::vector<cv::Mat> hsv;
		cv::split(*matProcessing, hsv);
		cv::Mat hue = hsv[0];
		cv::Mat saturation = hsv[1];
		cv::Mat value = hsv[2];
		
		// streets are detected by red pixels
		matStreets->create(value.size(), CV_8UC1);
		matStreets->setTo(cv::Scalar(0));
		
		// optimize vue channel
		cv::bitwise_not(value, value); // invert image for better thresholding
		for(int i = 0; i < value.rows; i++) {
			for(int j = 0; j < value.cols; j++) {
				
				// hsv values
				int valHue = hue.at<uchar>(i, j);
				int valSat = saturation.at<uchar>(i, j);
				int valVal = value.at<uchar>(i, j);
				
				// rgb values
				int r = (*matTracking).at<cv::Vec3b>(i, j)[2];
				int g = (*matTracking).at<cv::Vec3b>(i, j)[1];
				int b = (*matTracking).at<cv::Vec3b>(i, j)[0];

				// extract red pixels for streets
				float range = thresholdRed/255.0 * 50;
				bool isRed = r > 50 && (valHue < range || valHue > 180 - range) && valVal > 50 && valSat > 50;
				if(isRed) { 
					matStreets->at<uchar>(i, j) = 255; 
					value.at<uchar>(i, j) = 0;
				}

				// remove gray pixels
				else if(r > 150 && abs(r - g) < 10 &&abs(g - b) < 20) { value.at<uchar>(i, j) = 0; }
				// remove dark pixels
				else if(valVal < thresholdDark && valSat < thresholdDark) { value.at<uchar>(i, j) = 0; }
				// add light pixels
				else if(valVal > thresholdLight) { value.at<uchar>(i, j) = 255; }
				// prefer saturation if higher than value
				else if(valSat > valVal && valSat > 70 && valVal > 60) { value.at<uchar>(i, j) = 255; }
			}
		}

		// optimize streets channel and remove red pixels from value channel
		cv::Mat kernelDilate = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5,5));
		cv::medianBlur(*matStreets, *matStreets, 3);
		cv::dilate(*matStreets, *matStreets, kernelDilate);
		cv::subtract(value, *matStreets, value);

		if(renderMode == RenderMode::PaperScope && viewMode == PSViewMode::Processing) {
			cv::cvtColor(value, *matRender, cv::COLOR_GRAY2BGR);
		}

		drawHistogram(value);

		// combine rendering of processing with red channel for streets
		if(renderMode == RenderMode::PaperScope && viewMode == PSViewMode::Processing) {
			cv::Mat mask = cv::Mat::zeros(matTracking->size(), CV_8UC3);
			cv::cvtColor(*matStreets, mask, cv::COLOR_GRAY2BGR);
			cv::bitwise_and(*matTracking, mask, mask);
			cv::bitwise_or(*matRender, mask, *matRender);
		}

		*matProcessing = value.clone();
	}


	void PSDetect::whiteBalance(cv::Mat &mat) {
		
		// get reference white
		cv::Rect rect = cv::Rect(160, 20, 30, 30);
		cv::Mat roi = mat(rect);
		cv::Scalar white = cv::mean(roi);

		// calculate scaling factor
		float scaleR = white[2] / white[1];
		float scaleB = white[2] / white[0];

		// apply scaling factor
		for(int i = 0; i < mat.rows; i++) {
			for(int j = 0; j < mat.cols; j++) {
				mat.at<cv::Vec3b>(i, j)[0] = cv::saturate_cast<uchar>(mat.at<cv::Vec3b>(i, j)[0] * scaleB);
				mat.at<cv::Vec3b>(i, j)[1] = cv::saturate_cast<uchar>(mat.at<cv::Vec3b>(i, j)[1] * 1.0);
				mat.at<cv::Vec3b>(i, j)[2] = cv::saturate_cast<uchar>(mat.at<cv::Vec3b>(i, j)[2] * scaleR);
			}
		}

		if(renderMode == RenderMode::PaperScope && viewMode == PSViewMode::Plane2D) {
			*matRender = *matTracking;
			// draw white balance reference
			cv::rectangle(*matRender, rect, cv::Scalar(0, 128, 255), 2);
		}
	}


	void PSDetect::drawHistogram(cv::Mat &value) {

		// reshape value for histogram
		cv::Mat valueReshape = value.reshape(1, 1);

		// create histogram
		cv::Mat hist;
		int histSize = 256;
		float range[] = { 0, 256 };
		const float* histRange = { range };
		cv::calcHist(&valueReshape, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);

		// remove black/white pixels from histogram for better normalization
		for(int i = 0; i < 30; i++) { hist.at<float>(i) = 0; }
		for(int i = 235; i < 255; i++) { hist.at<float>(i) = 0; }

		// draw histogram to matRender
		if(renderMode == RenderMode::PaperScope && viewMode == PSViewMode::Processing) {

			int hist_w = 200;
			int hist_h = 180;
			int bin_w = cvRound((double) hist_w / histSize);
			cv::Mat histImage(hist_h, hist_w, CV_8UC3, cv::Scalar(20, 20, 20));
			cv::normalize(hist, hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());

			// draw histogram lines
			for(int i = 1; i < histSize; i++) {

				cv::line(histImage, cv::Point(bin_w * (i - 1), hist_h - cvRound(hist.at<float>(i - 1))),
					cv::Point(bin_w * (i), hist_h - cvRound(hist.at<float>(i))),
					i < thresholdDark || i > thresholdLight ? cv::Scalar(180, 50, 180) : cv::Scalar(255, 80, 0),
					2, 8, 0);
			}

			// add thresholds as vertical lines
			cv::line(histImage, cv::Point(thresholdDark, 0), cv::Point(thresholdDark, hist_h), cv::Scalar(180, 50, 180), 2, 8, 0);
			cv::line(histImage, cv::Point(thresholdLight, 0), cv::Point(thresholdLight, hist_h), cv::Scalar(180, 50, 180), 2, 8, 0);

			// add histogram to render
			cv::Rect roi = cv::Rect(0, 0, histImage.cols, histImage.rows);
			histImage.copyTo((*matRender)(roi));
		}
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DETECTION
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSDetect::applyThreshold() {

		// edge detection to find regions of interests
		cv::Mat edges;
		cv::Canny(*matProcessing, edges, 100, 180);
		cv::Mat kernelDilate = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
		cv::dilate(edges, edges, kernelDilate);

		// remove aruco marker edge
		cv::Rect roi = cv::Rect(0, 0, 240, 240);
		edges(roi) = cv::Scalar(0);

		// find contours of edges
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(edges, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

		*matThreshold = cv::Mat::zeros(edges.size(), CV_8UC1);

		// only outer contours of edges with a minimum area
		for(int i = 0; i < (int) contours.size(); i++) {
			
			if(hierarchy[i][3] == -1 && isValidContour(contours[i])) {

				// find bounding box of edge
				cv::Rect rect = cv::boundingRect(contours[i]);

				// otsu thresholding on bounding box
				cv::Mat roi = (*matProcessing)(rect);
				cv::GaussianBlur(roi, roi, cv::Size(5, 5), 0);
				cv::threshold(roi, roi, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
				roi.copyTo((*matThreshold)(rect));
			}
		}

		if(renderMode == RenderMode::PaperScope && viewMode == PSViewMode::Threshold) {
			cv::cvtColor(*matThreshold, *matRender, cv::COLOR_GRAY2BGR);
		}
	}


	void PSDetect::findContours() {

		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		candidates.clear();
		
		// find contours
		cv::findContours(*matThreshold, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
		if(contours.size() == 0) { return; }

		// only outer contours with a minimum area
		for(int i = 0; i < (int) contours.size(); i++) {
			
			if(hierarchy[i][3] == -1 && isValidContour(contours[i])) {

				// simplify contour
				cv::approxPolyDP(contours[i], contours[i], 0.02 * cv::arcLength(contours[i], true), true);
				
				createCandidate(contours[i]);
			}
		}
	}


	bool PSDetect::isValidContour(std::vector<cv::Point> contour) {

		// minimum size of contour
		if(contour.size() < 3) { return false; }
		if(cv::contourArea(contour) < 300) { return false; }

		// maximum size of contour
		if(cv::contourArea(contour) > 300*300) { return false; }

		// no extreme aspect ratio
		cv::RotatedRect rect = cv::minAreaRect(contour);
		float aspectRatio = rect.size.width / rect.size.height;
		if(aspectRatio > 10 || aspectRatio < 0.1) { return false; }

		return true;
	}


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DETECTION
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSDetect::findStreets() {

		// convert shapes to equal lines
		cv::Mat matThinning;
		cv::ximgproc::thinning(*matStreets, matThinning);

		if(renderMode == RenderMode::PaperScope && viewMode == PSViewMode::Streets) {
			cv::Mat channels[3];
			cv::split(*matRender, channels);
			channels[0] *= 0.15;
			channels[1] *= 0.15;
			channels[2] = matThinning;
			cv::merge(channels, 3, *matRender);
		}

		// equal width of streets
		cv::dilate(matThinning, *matStreets, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(6,6)));

		// find contours
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;
		cv::findContours(*matStreets, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
		if(contours.size() == 0) { return; }

		// filter contours
		for(int i = 0; i < (int) contours.size(); i++) {
			
            if(hierarchy[i][3] == -1 && cv::contourArea(contours[i]) > 300) {

				// simplify contour
				cv::approxPolyDP(contours[i], contours[i], 1, true);

				PSCandidate candidate(contours[i], PSShapeType::Street);
				candidates.push_back(candidate);
			}
		}
		
		if(renderMode == RenderMode::PaperScope && viewMode == PSViewMode::BoundingBoxes) {
			cv::add(*matThreshold, *matStreets, *matThreshold);
		}
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CANDIDATES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSDetect::createCandidate(std::vector<cv::Point> contour) {

		// classify shape
		cv::Rect rect = cv::boundingRect(contour);
		cv::Mat roi = (*matThreshold)(rect);
		PSShapeType shapeType = classifyShape(roi);

		// create candidate
		PSCandidate candidate(contour, shapeType);
		candidates.push_back(candidate);
	}


	void PSDetect::drawCandidates() {

		if(renderMode != RenderMode::PaperScope) { return; }

		// render threshold mat
		if(viewMode > PSViewMode::Streets) {
			cv::cvtColor(*matThreshold, *matRender, cv::COLOR_GRAY2BGR);
			cv::multiply(*matRender, 0.25, *matRender);
		}

		// render candidates bounding boxes
		if(viewMode == PSViewMode::BoundingBoxes) {
			for(int i = 0; i < (int) candidates.size(); i++) {
				candidates[i].drawBoundingBox(matRender);
			}
		}
	}
	


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AI
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSDetect::initAI() {

		// load model
		QString filePath = QCoreApplication::applicationDirPath() + "/shape-classifier_v4.tflite";
		model = tflite::FlatBufferModel::BuildFromFile(filePath.toStdString().c_str()); 

		// create interpreter
		resolver = new tflite::ops::builtin::BuiltinOpResolver();
		tflite::InterpreterBuilder(*model, *resolver)(&interpreter);
		interpreter->AllocateTensors();
	}


	PSShapeType PSDetect::classifyShape(cv::Mat roi) {

		// skip on empty roi
		if(roi.empty() || roi.cols==0 || roi.rows == 0) { return PSShapeType::Rectangle; }

		// resize image
		cv::resize(roi, roi, cv::Size(54, 54));

		// add padding of 3 pixels
		cv::Mat padded = cv::Mat(64, 64, CV_8UC1, cv::Scalar(0));
		roi.copyTo(padded(cv::Rect(5, 5, roi.cols, roi.rows)));

		// convert to float
		cv::Mat floatImg;
		padded.convertTo(floatImg, CV_32FC1, 1.0 / 255.0);

		// set input tensor
		float *input = interpreter->typed_input_tensor<float>(0);
		for(int i = 0; i < 64; i++) {
			for(int j = 0; j < 64; j++) {
				input[i * 64 + j] = floatImg.at<float>(i, j);
			}
		}

		// run inference
		interpreter->Invoke();

		// get output tensor
		float *output = interpreter->typed_output_tensor<float>(0);
		
		int maxIndex = 0;
		float maxValue = output[0];
		for(int i = 1; i < 5; i++) {
			if(output[i] > maxValue) {
				maxIndex = i;
				maxValue = output[i];
			}
		}

		return (PSShapeType) maxIndex;
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DATASET CAPTURE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSDetect::saveDataset(cv::Rect rect) {

		// save dataset
		cv::Mat roi = (*matThreshold)(rect);
		cv::resize(roi, roi, cv::Size(54, 54));

		// add padding of 3 pixels
		cv::Mat padded = cv::Mat(64, 64, CV_8UC1, cv::Scalar(0));
		roi.copyTo(padded(cv::Rect(5, 5, roi.cols, roi.rows)));

		// get desktop path
		QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
		desktopPath = desktopPath + "/paperscope_dataset/raw/";

		std::string filename = desktopPath.toStdString() + QString::number(QRandomGenerator::global()->generate()).toStdString() + ".png";
		cv::imwrite(filename, padded);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SETTINGS
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSDetect::onSettingsUpdated(QString key, QVariant value) {

		if(key == "renderMode") {
			renderMode = (RenderMode) value.toInt();
		}
		else if(key == "paperscope_viewmode") {
			viewMode = (PSViewMode) value.toInt();
		}
		else if(key == "threshold_dark") {
			thresholdDark = value.toInt() < thresholdLight ? value.toInt() : thresholdLight - 1;
		}
		else if(key == "threshold_light") {
			thresholdLight = value.toInt() > thresholdDark ? value.toInt() : thresholdDark + 1;
		}
		else if(key == "threshold_red") {
			thresholdRed = value.toInt();
		}
		else if(key == "capture_dataset") {
			captureDataset = true;
		}
	}

