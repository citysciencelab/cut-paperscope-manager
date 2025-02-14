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

	// App
	#include "../PSTrackingMode.h"
	#include "../../global/Settings.h"
	#include "../../ui/renderer/RenderMode.h"



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CLASS DECLARATION
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


class PSCapture : public QObject {
	
	Q_OBJECT

	public:
		
		explicit PSCapture(QObject *parent = nullptr);
		~PSCapture();

		// processing
		void init();
		void update(cv::Mat *mTracking, cv::Mat *mRender, PSTrackingMode trackingMode);
		void close();

		// opencv
		cv::VideoCapture *capture;	
		cv::Mat *matTracking;
		cv::Mat *matRender;

		// calibration
		std::vector<cv::Point2f> manualImagePoints;
		std::vector<cv::Point2f> currentImagePoints;


	private:

		// image processing
		void processImage();

		// aruco marker
		void findArucoMarker(int id = 16);
		void drawArucoMarker();
		void estimateMarkerPose();
		float markerPadding;
		float markerSize;
		std::vector<int> markerIds;
		std::vector<std::vector<cv::Point2f>> markerCorners;

		// 2d plane
		void get2DPlane();
		void updatePlaneSize();
		float planeWidth;
		float planeHeight;

		// smoothing
		void applySmoothing(std::vector<cv::Point2f> points);
		float smoothingFactor;
		float scalingFactor;

		// camera
		void openCamera();
		void setCameraProps();
		cv::Mat cameraMatrix;
		cv::Mat distCoeffs;
		std::vector<cv::Vec3d> tvecs;
		std::vector<cv::Vec3d> rvecs;

		// calibration
		void loadCameraCalibration();
		QString calibrationMode;

		// renderer
		RenderMode renderMode;

		// project
		QJsonObject project;

	
	public slots:

		// settings
		void onSettingsUpdated(QString key, QVariant value);
};

