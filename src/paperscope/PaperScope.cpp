/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#include "PaperScope.h"

	// Qt
	#include <QApplication>
	#include <QThread>



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONSTRUCTOR
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	PaperScope::PaperScope(QObject *parent)
		: QThread(parent),
		  psCapture(nullptr),
		  psCalibrate(nullptr),
		  psDetect(nullptr),
		  psDescribe(nullptr),
		  matTracking(nullptr),
		  trackingMode(PSTrackingMode::None),
		  matRender(nullptr),
		  renderMode(RenderMode::Camera)
	{

	}


	PaperScope::~PaperScope() {

		delete psCapture;
		delete psCalibrate;
		delete psDetect;
		delete psDescribe;

		delete matTracking;
		delete matRender;
	}
	


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PROCESSING
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PaperScope::run() {
		
		psCapture = new PSCapture();
		psCalibrate = new PSCalibrate();
		psDetect = new PSDetect();
		psDescribe = new PSDescribe();

		matTracking = new cv::Mat();
		matRender = new cv::Mat();

		initSettings();

		// connect signals
		connect(psCalibrate, &PSCalibrate::calibrationCompleted, this, &PaperScope::stopCalibrate);

		// keep thread alive
		isClosing = false;
		exec();
	}	


	int PaperScope::exec() {

		while(!isClosing) {
			
			// skip loop if not tracking
			if(trackingMode == PSTrackingMode::None) { 
				msleep(2000); 
				continue; 
			}

			startFpsTimer();
			
			// update paperscope
			psCapture->update(matTracking, matRender, trackingMode);
			bool wait = psCalibrate->update(matTracking, matRender, trackingMode);
			psDetect->update(matTracking, matRender, trackingMode);
			psDescribe->update(matTracking, matRender, psDetect->matStreets, trackingMode, psDetect->candidates);

			// finish loop
			updateRenderer();
            if(wait) { QThread::msleep(2000); }
		}

		// finish thread
		psCapture->close();
		psCalibrate->close();
		psDetect->close();
		psDescribe->close();

		return 0;
	}


	void PaperScope::close() {

		isClosing = true;
		wait();
		exit();
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	TRACKING
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PaperScope::startPreview() {

		PSTrackingMode newMode = PSTrackingMode::Preview;

		// close old cam first
		if(trackingMode != PSTrackingMode::None) { 
			
			newMode = trackingMode;
			changeTrackingMode(PSTrackingMode::None);

			QThread::msleep(1000);
		}

		psCapture->init();

		changeTrackingMode(newMode);
	}


	void PaperScope::startTracking() {

		// update state
		if(trackingMode == PSTrackingMode::Tracking) { return; }
		if(trackingMode == PSTrackingMode::None) { startPreview(); }
		
		changeTrackingMode(PSTrackingMode::Tracking);
	}


	void PaperScope::stopTracking() {

		if(trackingMode == PSTrackingMode::Stopped) { return; }

		changeTrackingMode(PSTrackingMode::Stopped);
	}


	void PaperScope::startCalibrate() {

		if(trackingMode == PSTrackingMode::Calibrate) { return; }
		psCalibrate->init();

		changeTrackingMode(PSTrackingMode::Calibrate);
	}


	void PaperScope::stopCalibrate() {

		if(trackingMode == PSTrackingMode::Stopped) { return; }
		psCalibrate->close();
		
		changeTrackingMode(PSTrackingMode::Stopped);
	}


	void PaperScope::changeTrackingMode(PSTrackingMode newMode) {

		if(trackingMode == newMode) { return; }
		
		PSTrackingMode oldMode = trackingMode;
		trackingMode = newMode;
		
		emit trackingModeChanged(newMode, oldMode);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	RENDERER
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */
			

	void PaperScope::updateRenderer() {

		if(renderMode == RenderMode::Camera || renderMode == RenderMode::PaperScope) {
			drawFpsTimer();	
			emit updated(*matRender, psCapture->currentImagePoints);
		}
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SETTINGS
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PaperScope::initSettings() {

		connect(Settings::instance(), &Settings::settingsUpdated, this, &PaperScope::onSettingsUpdated);
	}


	void PaperScope::onSettingsUpdated(QString key, QVariant value) {

		// sub classes need manual update
		psCapture->onSettingsUpdated(key, value);
		psDetect->onSettingsUpdated(key, value);
		psDescribe->onSettingsUpdated(key, value);

		if(key == "renderMode") { 
			renderMode = (RenderMode) value.toInt();
		}	
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	FPS
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PaperScope::startFpsTimer() {

		fpsTick = cv::getTickCount();
	}


	void PaperScope::drawFpsTimer() {

		if(!matRender || trackingMode != PSTrackingMode::Tracking) { return; }

		// generate fps string output		
		int fps = (int) cv::getTickFrequency() / (cv::getTickCount() - fpsTick);
		std::string output = "FPS: " + std::to_string(fps);

		cv::putText(*matRender, output, cv::Point(30, 40), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
	}
