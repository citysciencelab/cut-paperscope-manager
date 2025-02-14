/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#include "MainNavi.h"

	// Qt
	#include <QSettings>
	#include <QCameraFormat>
	#include <QTimer>

	// App
    #include "../../MainWindow.h"
    #include "../../global/Settings.h"
    #include "../../global/Api.h"
    #include "../../global/Broadcast.h"



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONSTRUCTOR
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	MainNavi::MainNavi(QWidget *parent)
		: QWidget(parent),
		  layout(nullptr),
		  logo(nullptr),
		  labelCameraDevice(nullptr),
		  selectCameraDevice(nullptr),
		  selectCameraFormat(nullptr),
		  btnCamera(nullptr),
		  separator(nullptr),
		  paperScope(nullptr),
		  labelPaperScope(nullptr),
		  btnRealtime(nullptr),
		  btnSnapshot(nullptr)
	{
	
		// init properties
		setFixedWidth(280);

		// save paperscope pointer
		MainWindow *mainWindow = dynamic_cast<MainWindow*>(parent);
		paperScope = mainWindow->paperScope;

		// init member
		initLayout();
		initHeader();
		initProject();
		initCamera();
		initPaperScope();	
		initSettings();
	}


	MainNavi::~MainNavi() {

		delete layout;
		delete logo;
		
		delete labelCameraDevice;
		delete selectCameraDevice;
		delete selectCameraFormat;
		delete btnCamera;

		delete btnRealtime;
		delete btnSnapshot;
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	LAYOUT
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void MainNavi::initLayout() {

		layout = new QVBoxLayout();
		setLayout(layout);

		// init layout properties
		layout->setContentsMargins(10,0,10,0);
		layout->setAlignment(Qt::AlignTop);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	HEADER
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void MainNavi::initHeader() {

		logo = new QSvgWidget(":/svg/header-logo.svg",this);
		logo->setFixedSize(280,50);
		layout->addWidget(logo);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PROJECT
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void MainNavi::initProject() {

		// label
		labelProject = new QLabel("Project",this);
		labelProject->setObjectName("title");
		layout->addWidget(labelProject);

		// input
		inputProject = new QLineEdit(this);
		inputProject->setPlaceholderText("Enter project ID");
		inputProject->setText(Settings::instance()->getString("project_id").toUpper());
		inputProject->setMaxLength(9);
		layout->addWidget(inputProject);
		connect(inputProject, &QLineEdit::editingFinished, this, &MainNavi::onProjectInputSubmit);
		connect(inputProject, &QLineEdit::textChanged, this, &MainNavi::onProjectInputChanged);

		// socket status
		labelSocketStatus = new QLabel("No project loaded",this);
		labelSocketStatus->setObjectName("status-label");
		labelSocketStatus->setAlignment(Qt::AlignCenter);
		layout->addWidget(labelSocketStatus);

		// separator
		separator = new QSvgWidget(":/svg/icons/separator.svg",this);
		separator->setFixedSize(280,30);
		layout->addWidget(separator);

		// connect signals
		connect(Api::instance(), &Api::error, this, &MainNavi::onProjectError);

		// load from settings
		QString projectId = Settings::instance()->getString("project_id");
		inputProject->setText(projectId);
	}


	void MainNavi::onProjectInputChanged() {
		
		style()->polish(inputProject);

		// transform input to uppercase
		QString project = inputProject->text().toUpper();
		inputProject->setText(project);	
	}


	void MainNavi::onProjectInputSubmit() {

		QString project = inputProject->text();
		inputProject->clearFocus();

		if(project.length() > 3) { loadProject(project); }
	}


	void MainNavi::onProjectError(QString message, int code, QString url) {

		if(url.contains("api/project")) {
			labelSocketStatus->setText("Project not found");
			labelSocketStatus->setStyleSheet("color: #FB334F;");
		}
		else {
			labelSocketStatus->setText("Connection error");
			labelSocketStatus->setStyleSheet("color: #FB334F;");
		}
	}


	void MainNavi::loadProject(QString projectId) {

		labelSocketStatus->setText("Loading...");
		labelSocketStatus->setStyleSheet("color: inherit;");

        Api::instance()->get("api/project/"+projectId.toLower(), [=](QJsonObject data) {

			labelSocketStatus->setText("Project loaded");
			
			// save projet
			Settings::instance()->saveJsonObject("project", data);
			Settings::instance()->saveString("project_id", data.value("slug").toString());

			// subscribe to websocket
			connect(Broadcast::instance(), &Broadcast::connected, [=](){
			
           		Broadcast::instance()->subscribePrivateChannel("project."+projectId.toLower());
			});
		});
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CAMERA
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void MainNavi::initCamera() {

		// select device
		labelCameraDevice = new QLabel("Camera",this);
		labelCameraDevice->setObjectName("title");
		selectCameraDevice = new QComboBox(this);
		selectCameraDevice->setCursor(Qt::PointingHandCursor);
		layout->addWidget(labelCameraDevice); 
		layout->addWidget(selectCameraDevice);

		// select format
		selectCameraFormat = new QComboBox(this);
		selectCameraFormat->setCursor(Qt::PointingHandCursor);
		layout->addWidget(selectCameraFormat);

		// button camera
		btnCamera = new QPushButton("Open camera",this);
		btnCamera->setCursor(Qt::PointingHandCursor);
		layout->addWidget(btnCamera);
		connect(btnCamera, &QPushButton::pressed, paperScope, &PaperScope::startPreview);

		// separator
		separator = new QSvgWidget(":/svg/icons/separator.svg",this);
		separator->setFixedSize(280,30);
		layout->addWidget(separator);
		
		// connect signals
		MainWindow *mainWindow = dynamic_cast<MainWindow*>(parent());
		connect(mainWindow, &MainWindow::cameraDevicesUpdated, this, &MainNavi::initCameraDevices);
	}
	

	void MainNavi::initCameraDevices(QList<QCameraDevice> cameras) {

		cameraDevices = cameras;

		// set items of select input
		selectCameraDevice->clear();
		for(auto &camera : cameras) { selectCameraDevice->addItem(camera.description()); }

		// find a saved camera device
		QString cameraDevice = Settings::instance()->getString("cameraDevice");
		int index = cameraDevice.isEmpty() ? 0 : selectCameraDevice->findText(cameraDevice);
		if(index < 0) { index = 0; }
		
		// set active camera device
		selectCameraDevice->setCurrentIndex(index);
		if(cameraDevice.isEmpty()) {
			Settings::instance()->saveString("cameraDevice", cameraDevices.at(0).description());
		}

		// set camera formats
		setCameraFormats(cameras.at(index));

		// activate saved camera format
		QString cameraFormat = Settings::instance()->getString("cameraFormat", "1280x720 - 60fps (NV12)");
		index = selectCameraFormat->findText(cameraFormat);
		selectCameraFormat->setCurrentIndex(index < 0 ? 0 : index);

		// enable camera buttons
		connect(selectCameraDevice, &QComboBox::currentIndexChanged, this, &MainNavi::onCameraDeviceSelected);
		connect(selectCameraFormat, &QComboBox::currentIndexChanged, this, &MainNavi::onCameraFormatSelected);
	}


	void MainNavi::setCameraFormats(const QCameraDevice camera) {

		QList<QCameraFormat> formats = camera.videoFormats();

		selectCameraFormat->clear();
		for(auto &format : formats) { 

			QSize size = format.resolution();
			if(size.width() < 900) { continue; }

			// pixel resolution
			QString label = QString::number(size.width()) + "x" + QString::number(size.height());
			
			// fps
			label += " - " + QString::number((int)format.maxFrameRate()) + "fps";
			
			// pixel format
			if(format.pixelFormat() != QVideoFrameFormat::Format_YUYV) { label += " (YUYV)"; }
			else if(format.pixelFormat() != QVideoFrameFormat::Format_NV12) { label += " (NV12)"; }

			selectCameraFormat->addItem(label);
		}	
	}


	void MainNavi::onCameraDeviceSelected(int index) {

		QString value = cameraDevices.at(index).description();
		Settings::instance()->saveString("cameraDevice", value);

		setCameraFormats(cameraDevices.at(index));
	}


	void MainNavi::onCameraFormatSelected(int index) {

		QString value = selectCameraFormat->itemText(index);
		Settings::instance()->saveString("cameraFormat", value);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PAPERSCOPE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void MainNavi::initPaperScope() {

		// label
		labelPaperScope = new QLabel("PaperScope",this);
		labelPaperScope->setObjectName("title");
		layout->addWidget(labelPaperScope);

		// button realtime
		btnRealtime = new QPushButton("Start Realtime",this);
		btnRealtime->setObjectName("btnRealtime");
		btnRealtime->setCursor(Qt::PointingHandCursor);
		btnRealtime->setCheckable(true);
		layout->addWidget(btnRealtime);

		// button snapshot
		btnSnapshot = new QPushButton("Take Snapshot",this);
		btnSnapshot->setObjectName("btnSnapshot");
		btnSnapshot->setCursor(Qt::PointingHandCursor);
		layout->addWidget(btnSnapshot);
		
		// connect signals
		connect(btnRealtime, &QPushButton::clicked, this, &MainNavi::onBtnRealtimeClicked);
		connect(btnSnapshot, &QPushButton::clicked, this, &MainNavi::onBtnSnapshotClicked);
        connect(paperScope, &PaperScope::trackingModeChanged, this, &MainNavi::onTrackingModeChanged);
	}


	void MainNavi::onBtnRealtimeClicked() {

		if(!paperScope || !btnRealtime->isEnabled()) { return; }

		if(btnRealtime->isChecked()) { 
			btnSnapshot->setEnabled(false);
			btnRealtime->setText("Stop Realtime");
			paperScope->startTracking();
		}
		else {
			btnSnapshot->setEnabled(true);
			btnRealtime->setText("Start Realtime");
			paperScope->stopTracking();
		}
	}


	void MainNavi::onBtnSnapshotClicked() {

		if(!paperScope || !btnSnapshot->isEnabled()) { return; }

		btnSnapshot->setEnabled(false);

		QString projectId = Settings::instance()->getString("project_id");

		// hide beamer
		QJsonObject data;
		data["value"] = true;
		Broadcast::instance()->sendPrivateChannel("project."+projectId, "ToggleBeamer", data);

		// automatic stop after 4 seconds
		QTimer::singleShot(5000, [this](){ 

			paperScope->startTracking();
			
			QTimer::singleShot(5000, [this](){ 

				paperScope->stopTracking();
				btnSnapshot->setEnabled(true); 
				QString projectId = Settings::instance()->getString("project_id");

				// show beamer
				QJsonObject data;
				data["value"] = false;
				Broadcast::instance()->sendPrivateChannel("project."+projectId, "ToggleBeamer", data);
			});
		});
	}


	void MainNavi::onTrackingModeChanged(PSTrackingMode newMode, PSTrackingMode oldMode) {

		if(newMode == PSTrackingMode::Calibrate) {
			btnSnapshot->setEnabled(false);
			btnRealtime->setEnabled(false);
			btnRealtime->setChecked(false);
			btnRealtime->setText("Start Realtime");
		}
		else if(oldMode == PSTrackingMode::Calibrate) {
			btnSnapshot->setEnabled(true);
			btnRealtime->setEnabled(true);
		}
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SETTINGS
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void MainNavi::initSettings() {

		connect(Settings::instance(), &Settings::settingsUpdated, this, &MainNavi::onSettingsUpdated);
	}


	void MainNavi::onSettingsUpdated(QString key, QVariant value) {

		if(key == "project") { 
			labelSocketStatus->setText("Project loaded");
 		}	
	}
