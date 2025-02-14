/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#include "Renderer.h"

	// App
	#include "../../MainWindow.h"
	#include "../../global/Settings.h"



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONSTRUCTOR
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	Renderer::Renderer(QWidget *parent)
		: QWidget(parent),
		  layout(nullptr),
		  tabBar(nullptr),
		  renderMode(RenderMode::Camera),
		  cameraModeConfig(nullptr),
          calibrationMode("auto"),
		  cornerPointTL(nullptr),
		  cornerPointTR(nullptr),
		  cornerPointBR(nullptr),
		  cornerPointBL(nullptr),
		  paperScopeModeConfig(nullptr),
		  paperScope(nullptr),
		  preferencesTab(nullptr)
	{

		// init properties
		isCalibrating = false;

		// save paperscope pointer
		MainWindow *mainWindow = dynamic_cast<MainWindow*>(parent);
		paperScope = mainWindow->paperScope;

		// init member
		initSettings();
		initUserInterface();
		initRenderer();
		initCamera();	
		initPaperScope();	
		initPreferences();
	}


	Renderer::~Renderer() {

		delete layout;
		delete tabBar;

		delete cameraModeConfig;
		delete paperScopeModeConfig;

		delete cornerPointTL;
		delete cornerPointTR;
		delete cornerPointBR;
		delete cornerPointBL;
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	USER INTERFACE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void Renderer::initUserInterface() {

		// layout
		layout = new QVBoxLayout();
		layout->setContentsMargins(0,15,0,0);
		layout->setSpacing(10);
		setLayout(layout);

		// tabs
		tabBar = new QTabBar();
		tabBar->addTab("Camera");
		tabBar->addTab("PaperScope");
		tabBar->addTab("Preferences");
		layout->addWidget(tabBar);
		layout->setAlignment(tabBar, Qt::AlignCenter);

		// events
		connect(tabBar, &QTabBar::currentChanged, this, &Renderer::onTabChanged);
	}


	void Renderer::onTabChanged(int index) {

		switch(index) {
			case 0: renderMode = RenderMode::Camera; break;
			case 1: renderMode = RenderMode::PaperScope; break;
            case 2: renderMode = RenderMode::Preferences; break;
		}

		Settings::instance()->saveInt("renderMode", (int)renderMode);

		// update ui
		renderView.setVisible(renderMode == RenderMode::Camera || renderMode == RenderMode::PaperScope);
		cameraModeConfig->setVisible(renderMode == RenderMode::Camera);
        paperScopeModeConfig->setVisible(renderMode == RenderMode::PaperScope);
        preferencesTab->setVisible(renderMode == RenderMode::Preferences);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PROCESSING
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	/**
	 * A loop function that gets called every frame to update ui and renderer. Triggerd by PaperScope::updated signal.
	 */

	void Renderer::update(cv::Mat mat, std::vector<cv::Point2f> points) {

		if(renderMode == RenderMode::Camera) {
			updateRenderer(mat, points);
		}
		else if(renderMode == RenderMode::PaperScope) {
			updateRenderer(mat, points);
		}
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	RENDERER
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void Renderer::initRenderer() {

		renderView.setFixedSize(720,405);
		renderView.setBackgroundBrush(QColor::fromRgb(250,250,250));
		layout->addWidget(&renderView);

		renderScene.addItem(&renderPixmapItem);

		renderView.setScene(&renderScene);
		renderView.setSceneRect(0,0,720,405);
		renderView.setInteractive(true);
	}


	void Renderer::updateRenderer(cv::Mat &mat, std::vector<cv::Point2f> points) {

		if(mat.empty()) { return; }

		int w = mat.cols;
		int h = mat.rows;
		int ww = renderView.width();
		int wh = renderView.height();

		// scale image to fit viewport in cover mode (720x405)
		QImage image = QImage(mat.data, w, h, QImage::Format_RGB888).rgbSwapped();
		if(w/h < ww/wh) { 
			scaling = ww*1.0f/w;
			w = ww; 
			h *= scaling; 
		}
		else { 
			scaling = wh*1.0f/h;
			h = wh; 
			w *= scaling; 
		}
		if(w==0 || h==0) { return; }

		image = image.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);

		// draw lines between corner points
		if(renderMode == RenderMode::Camera) {

			QPainter painter(&image);
			painter.setPen(QPen(QColor::fromRgb(255,255,0), 1));

			if(!isCalibrating && points.size() == 4) {
				
				// scale points
				for(int i=0; i<points.size(); i++) {
					points[i].x *= scaling;
					points[i].y *= scaling;
				}

				painter.drawLine(QPointF(points[0].x, points[0].y), QPointF(points[1].x, points[1].y));
				painter.drawLine(QPointF(points[1].x, points[1].y), QPointF(points[2].x, points[2].y));
				painter.drawLine(QPointF(points[2].x, points[2].y), QPointF(points[3].x, points[3].y));
				painter.drawLine(QPointF(points[3].x, points[3].y), QPointF(points[0].x, points[0].y));
			}
			else if(calibrationMode == "manual") {
                
				painter.drawLine(cornerPointTL->pos() + QPointF(15,15), cornerPointTR->pos() + QPointF(-5,15));
                painter.drawLine(cornerPointTR->pos() + QPointF(-5,15), cornerPointBR->pos() + QPointF(-5,-5));
				painter.drawLine(cornerPointBR->pos() + QPointF(-5,-5), cornerPointBL->pos() + QPointF(15,-5));
				painter.drawLine(cornerPointBL->pos() + QPointF(15,-5), cornerPointTL->pos() + QPointF(15,15));
			}
		}

		// draw image
		renderPixmapItem.setPixmap(QPixmap::fromImage(image));
		renderPixmapItem.setOffset((ww-w)/2, (wh-h)/2);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CAMERA
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void Renderer::initCamera() {

		cameraModeConfig = new CameraModeConfig(paperScope, this);
		cameraModeConfig->show();
		layout->addWidget(cameraModeConfig);

		// calibration points
        cornerPointTL = new DraggableRectItem();
		renderScene.addItem(cornerPointTL);

        cornerPointTR = new DraggableRectItem();
		renderScene.addItem(cornerPointTR);

        cornerPointBR = new DraggableRectItem();
		renderScene.addItem(cornerPointBR);

        cornerPointBL = new DraggableRectItem();
		renderScene.addItem(cornerPointBL);

		connect(cameraModeConfig, &CameraModeConfig::autoCalibrateStarted, paperScope, &PaperScope::startCalibrate);
		connect(cameraModeConfig, &CameraModeConfig::autoCalibrateStopped, paperScope, &PaperScope::stopCalibrate);

		connect(cameraModeConfig, &CameraModeConfig::manualCalibrateStarted, this, &Renderer::startManualCalibrate);
		connect(cameraModeConfig, &CameraModeConfig::manualCalibrateSaved, this, &Renderer::saveManualCalibrate);
		connect(cameraModeConfig, &CameraModeConfig::manualCalibrateReset, this, &Renderer::resetManualCalibrate);
		connect(cameraModeConfig, &CameraModeConfig::manualCalibrateStopped, this, &Renderer::stopManualCalibrate);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	MANUAL CALIBRATE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void Renderer::startManualCalibrate() {
		qDebug() << "start manual calibrate";
		isCalibrating = true;
		calibrationMode = "manual";

		std::vector<cv::Point2f> points = Settings::instance()->getPoints("calibration_points");

		// init corner points
		if(points.size() == 4) {
			cornerPointTL->setPos(points[0].x * scaling - 15, points[0].y * scaling - 15);
			cornerPointTR->setPos(points[1].x * scaling + 5, points[1].y * scaling - 15);
			cornerPointBR->setPos(points[2].x * scaling + 5, points[2].y * scaling + 5);
			cornerPointBL->setPos(points[3].x * scaling - 15, points[3].y * scaling + 5);
		}
		else {
			resetManualCalibrate();
		}

		cornerPointTL->setVisible(true);
		cornerPointTR->setVisible(true);
		cornerPointBR->setVisible(true);
		cornerPointBL->setVisible(true);
	}


	void Renderer::saveManualCalibrate() {

		isCalibrating = false;

		std::vector<cv::Point2f> points;
		points.push_back(cv::Point2f( (cornerPointTL->pos().x() + 15.0), (cornerPointTL->pos().y() + 15.0) ));
		points.push_back(cv::Point2f( (cornerPointTR->pos().x() - 5.0), (cornerPointTR->pos().y() + 15.0) ));
		points.push_back(cv::Point2f( (cornerPointBR->pos().x() - 5.0), (cornerPointBR->pos().y() - 5.0) ));
		points.push_back(cv::Point2f( (cornerPointBL->pos().x() + 15.0), (cornerPointBL->pos().y() - 5.0) ));

		// scale points
		for(int i=0; i<points.size(); i++) {
			points[i].x /= scaling;
			points[i].y /= scaling;
		}

        Settings::instance()->savePoints("calibration_points", points);

		// hide corner points
		cornerPointTL->setVisible(false);
		cornerPointTR->setVisible(false);
		cornerPointBR->setVisible(false);
		cornerPointBL->setVisible(false);

		paperScope->psCapture->currentImagePoints = points;
	}


	void Renderer::resetManualCalibrate() {
		
		cornerPointTL->setPos(50,50);
		cornerPointTR->setPos(620,50);
		cornerPointBR->setPos(620,305);
		cornerPointBL->setPos(50,305);
	}


	void Renderer::stopManualCalibrate() {

		isCalibrating = false;

		calibrationMode = Settings::instance()->getString("calibration_mode");

		// hide corner points
		cornerPointTL->setVisible(false);
		cornerPointTR->setVisible(false);
		cornerPointBR->setVisible(false);
		cornerPointBL->setVisible(false);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PAPERSCOPE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void Renderer::initPaperScope() {

		paperScopeModeConfig = new PaperScopeModeConfig(paperScope, this);
        paperScopeModeConfig->hide();
		layout->addWidget(paperScopeModeConfig);

		connect(paperScope, &PaperScope::updated, this, &Renderer::update);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INFO
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void Renderer::initPreferences() {

		preferencesTab = new PreferencesTab(this);
		preferencesTab->hide();

        layout->addWidget(preferencesTab);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SETTINGS
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void Renderer::initSettings() {

		connect(Settings::instance(), &Settings::settingsUpdated, this, &Renderer::onSettingsUpdated);
	}


	void Renderer::onSettingsUpdated(QString key, QVariant value) {
		
		if(key == "calibration_mode") {
			calibrationMode = value.toString();
			isCalibrating = false;
		}
	}

