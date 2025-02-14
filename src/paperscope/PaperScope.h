/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#pragma once

	// Qt
	#include <QThread>
	#include <QObject>
	#include <QJsonObject>

	// OpenCV
	#include <opencv2/opencv.hpp>

	// App
	#include "PSTrackingMode.h"
	#include "capture/PSCapture.h"
	#include "capture/PSCalibrate.h"
	#include "detect/PSDetect.h"
	#include "describe/PSDescribe.h"
	#include "../global/Settings.h"
	#include "../ui/renderer/RenderMode.h"



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CLASS DECLARATION
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


class PaperScope : public QThread {

	Q_OBJECT

	public:

		explicit PaperScope(QObject *parent = nullptr);
		~PaperScope();

		// paperscope
		PSCapture *psCapture;
		PSCalibrate *psCalibrate;
		PSDetect *psDetect;
		PSDescribe *psDescribe;
		cv::Mat *matTracking;

		// processing
		void run() override;
		int exec();
		void close();

		// tracking
		void changeTrackingMode(PSTrackingMode newMode);


	private:
		
		// tracking
		bool isClosing;
		PSTrackingMode trackingMode;

		// renderer
		void updateRenderer();
		cv::Mat *matRender;
		RenderMode renderMode;

		// settings
		void initSettings();

		// fps
		void startFpsTimer();
		void drawFpsTimer();
		int64 fpsTick;


	signals:
		
		void updated(cv::Mat mat, std::vector<cv::Point2f> points);
		void trackingModeChanged(PSTrackingMode newMode, PSTrackingMode oldMode);


	public slots:

		// tracking
		void startPreview();
		void startTracking();
		void stopTracking();
		void startCalibrate();
		void stopCalibrate();

		// settings
		void onSettingsUpdated(QString key, QVariant value);
};

