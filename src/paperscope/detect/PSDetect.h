/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#pragma once

	// Qt
	#include <QObject>

	// OpenCV
	#include <opencv2/opencv.hpp>

	// Tensorflow
	#include "tensorflow/lite/kernels/register.h"
	#include "tensorflow/lite/model.h"
	#include "tensorflow/lite/interpreter.h"

	// App
	#include "../PSTrackingMode.h"
	#include "../PSViewMode.h"
	#include "PSCandidate.h"
	#include "PSShapeType.h"
	#include "PSShapeType.h"
	#include "../../global/Settings.h"
	#include "../../ui/renderer/RenderMode.h"



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CLASS DECLARATION
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


class PSDetect : public QObject {
	
	Q_OBJECT

	public:
		
		explicit PSDetect(QObject *parent = nullptr);
		~PSDetect();

		// processing
		void init();
		void update(cv::Mat *mTracking, cv::Mat *mRender, PSTrackingMode trackingMode);
		void close();

		// opencv
		cv::Mat *matTracking;
		cv::Mat *matProcessing;
		cv::Mat *matThreshold;
		cv::Mat *matRender;

		// image processing
		void imageProcessing();
		void whiteBalance(cv::Mat &mat);
		void drawHistogram(cv::Mat &value);

		// detection
		void applyThreshold();
		void findContours();
		bool isValidContour(std::vector<cv::Point> contour);
		int thresholdDark;
		int thresholdLight;
		int thresholdRed;

		// streets
		void findStreets();
		cv::Mat *matStreets;

		// candidates
		void createCandidate(std::vector<cv::Point> contour);
		void drawCandidates();
		std::vector<PSCandidate> candidates;


	private:
		
		// renderer
		RenderMode renderMode;
		PSViewMode viewMode;

		// ai
		void initAI();
        PSShapeType classifyShape(cv::Mat roi);
		tflite::impl::FlatBufferModel::Ptr model;
		tflite::ops::builtin::BuiltinOpResolver *resolver;
		std::unique_ptr<tflite::Interpreter> interpreter;

		// dataset capture
		void saveDataset(cv::Rect rect);
		bool captureDataset;

	
	public slots:

		// settings
		void onSettingsUpdated(QString key, QVariant value);
};

