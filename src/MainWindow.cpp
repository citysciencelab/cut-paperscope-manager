/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#include "MainWindow.h"

	// Qt
	#include <QApplication>
	#include <QMediaDevices>
	#include <QPermissions>
	#include <QMessageBox>

	// OpenCV
	#include <opencv2/opencv.hpp>

	// App
	#include "paperscope/describe/PSDescribe.h"
	#include "global/Api.h"
	#include "global/Settings.h"



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONSTRUCTOR
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	MainWindow::MainWindow(QWidget *parent)
		: QMainWindow(parent),
		  paperScope(nullptr),
		  centralWidget(nullptr),
		  centralLayout(nullptr),
		  mainMenu(nullptr),
		  mainNavi(nullptr),
		  renderer(nullptr)
	{

		// init properties
		setFixedSize(1000,680);
		setWindowTitle("PaperScope");

		// init singletons
		Api::instance();
		Settings::instance();
		
		// init member
		initPaperScope();
		initUserInterface();	
		initCamera();

		show();
	}


	MainWindow::~MainWindow() {

		delete mainMenu;
		delete mainNavi;
		delete renderer;

		delete centralLayout;
		delete centralWidget;

		// no need to delete paperScope. Thread will be closed by itself.
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PAPERSCOPE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	/**
	 * Main entry point for PaperScope Controller
	 */

	void MainWindow::initPaperScope() {

		paperScope = new PaperScope();

		// init separate thread
		paperScope->start();
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	USER INTERFACE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	/**
	 * Load all ui widgets and add them to the main window. 
	 * 
	 * MainWindow needs a CentralWidget and CentralLayout as a basic setup.
	 */

	void MainWindow::initUserInterface() {

		centralWidget = new QWidget(this);
		setCentralWidget(centralWidget);
	
		centralLayout = new QHBoxLayout(centralWidget);
		centralLayout->setAlignment(Qt::AlignTop);
		centralLayout->setContentsMargins(0,0,0,0);
		centralLayout->setSpacing(0);

		mainMenu = new MainMenu(this);
		setMenuBar(mainMenu);

		mainNavi = new MainNavi(this);
		centralLayout->addWidget(mainNavi);

		renderer = new Renderer(this);
		centralLayout->addWidget(renderer);
		centralLayout->setAlignment(renderer, Qt::AlignTop);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CAMERA
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	/**
	 * Ask for camera permission on application start.
	 */

	void MainWindow::initCamera() {

		#if QT_CONFIG(permissions)

		QCameraPermission camPermission;

		switch (qApp->checkPermission(camPermission)) {
			case Qt::PermissionStatus::Undetermined:	return qApp->requestPermission(camPermission, this, &MainWindow::onPermissionUpdated);
			case Qt::PermissionStatus::Denied:			return onCameraDenied();
			case Qt::PermissionStatus::Granted:			return onCameraAccess();
		}

		#endif
	}


	/**
	 * Callback for camera permission request.
	 */

	void MainWindow::onPermissionUpdated(const QPermission &permission) {

		if (permission.status() != Qt::PermissionStatus::Granted) { 
			return onCameraDenied(); 
		}

		onCameraAccess();
	}


	/**
	 * Permission granted, save available camera information.
	 */

	void MainWindow::onCameraAccess() {

		QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
		emit cameraDevicesUpdated(cameras);
	}


	void MainWindow::onCameraDenied() {

		QMessageBox msgBox;
		msgBox.setText("Application needs camera access to work properly.");
		msgBox.exec();
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	WINDOW
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void MainWindow::closeEvent(QCloseEvent *event) {

		if(paperScope) { paperScope->close(); }

		event->accept();
	}


