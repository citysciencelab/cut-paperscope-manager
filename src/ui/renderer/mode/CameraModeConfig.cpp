/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#include "CameraModeConfig.h"

	// Qt
	#include <QLabel>



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONSTRUCTOR
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	CameraModeConfig::CameraModeConfig(PaperScope *paperScope, QWidget *parent)
		: QWidget(parent),
		  layout(nullptr),
		  labelCalibrationMode(nullptr),
		  selectCalibrationMode(nullptr),
		  btnCalibrate(nullptr),
		  btnCalibrateSave(nullptr),
		  btnCalibrateReset(nullptr),
		  labelAutoCalibrate(nullptr),
		  btnAutoCalibrate(nullptr),
		  labelManualCalibrate(nullptr),
		  btnManualCalibrate(nullptr),
		  paperScope(paperScope)
	{
		
		// init properties
		calibrationMode = Settings::instance()->getString("calibration_mode", "auto");

		// init member
		initUserInterface();
		initCalibrationMode();
		initCalibrate();
		initAutoCalibrate();
		initManualCalibrate();
		initPaperScope();
	}


	CameraModeConfig::~CameraModeConfig() {
		
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	USER INTERFACE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void CameraModeConfig::initUserInterface() {

		// layout
		layout = new QGridLayout();
		layout->setContentsMargins(25,25,25,0);
		layout->setSpacing(10);
		layout->setColumnMinimumWidth(1, 30);
		layout->setRowMinimumHeight(1, 25);
		setLayout(layout);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONFIG
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void CameraModeConfig::initCalibrationMode() {

		labelCalibrationMode = new QLabel("Active Calibration Mode", this);
		labelCalibrationMode->setObjectName("small");

		selectCalibrationMode = new QComboBox(this);
		selectCalibrationMode->setObjectName("secondary");
		selectCalibrationMode->setFixedWidth(180);
		selectCalibrationMode->setCursor(Qt::PointingHandCursor);
		selectCalibrationMode->addItem("Calibration Matrix");
		selectCalibrationMode->addItem("Manual Calibration");
		selectCalibrationMode->setCurrentIndex(calibrationMode == "auto" ? 0 : 1);

		layout->addWidget(labelCalibrationMode, 1, 0, Qt::AlignLeft);
		layout->addWidget(selectCalibrationMode, 2, 0, Qt::AlignLeft);

		connect(selectCalibrationMode, &QComboBox::currentIndexChanged, this, &CameraModeConfig::changeCalibrationMode);
	}


	void CameraModeConfig::changeCalibrationMode(int index) {

		calibrationMode = index == 0 ? "auto" : "manual";
		Settings::instance()->saveString("calibration_mode", calibrationMode);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CALIBRATE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void CameraModeConfig::initCalibrate() {

		// main button
		btnCalibrate = new QPushButton("Calibrate", this);
		btnCalibrate->setObjectName("secondary");
		btnCalibrate->setCursor(Qt::PointingHandCursor);
		btnCalibrate->setCheckable(true);
		btnCalibrate->setFixedWidth(150);
		btnCalibrate->setVisible(false);

		// save button
		btnCalibrateSave = new QPushButton("Save", this);
		btnCalibrateSave->setCursor(Qt::PointingHandCursor);
		btnCalibrateSave->setFixedWidth(150);
		btnCalibrateSave->setVisible(false);

		// reset button
		btnCalibrateReset = new QPushButton("Reset", this);
		btnCalibrateReset->setCursor(Qt::PointingHandCursor);
		btnCalibrateReset->setFixedWidth(150);
		btnCalibrateReset->setVisible(false);
		
		layout->addWidget(btnCalibrateSave, 2, 1, Qt::AlignLeft);
		layout->addWidget(btnCalibrateReset, 2, 0, Qt::AlignRight);
		layout->addWidget(btnCalibrate, 0, 1, Qt::AlignRight);

		connect(btnCalibrate, &QPushButton::clicked, this, &CameraModeConfig::toggleCalibrate);
		connect(btnCalibrateSave, &QPushButton::clicked, this, &CameraModeConfig::saveCalibrate);
		connect(btnCalibrateReset, &QPushButton::clicked, this, &CameraModeConfig::resetManualCalibrate);
	}


	void CameraModeConfig::toggleCalibrate() {

		// update state
		if(isActiveCalibrate && calibrationMode == "manual") {
			return toggleManualCalibrate();
		}
		else if(isActiveCalibrate && calibrationMode == "auto") {
			return toggleAutoCalibrate();
		}
		isCalibrate = !isCalibrate;
		btnCalibrate->setText(isCalibrate ? "Close" : "Calibrate");

		// hide config
		labelCalibrationMode->setVisible(!isCalibrate);
		selectCalibrationMode->setVisible(!isCalibrate);

		// show buttons
		labelAutoCalibrate->setVisible(isCalibrate);
		btnAutoCalibrate->setVisible(isCalibrate);
		btnManualCalibrate->setVisible(isCalibrate);
		labelManualCalibrate->setVisible(isCalibrate);
	}


	void CameraModeConfig::saveCalibrate() {

		if(calibrationMode == "auto") {
		}
		else {

			emit manualCalibrateSaved();
			toggleManualCalibrate();

			Settings::instance()->saveString("calibration_mode", "manual");
		}	

		toggleCalibrate();
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	AUTO CALIBRATE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void CameraModeConfig::initAutoCalibrate() {
		
		// label
		labelAutoCalibrate = new QLabel("Use a checkerboard pattern for calibration", this);
		labelAutoCalibrate->setObjectName("small");
		labelAutoCalibrate->setAlignment(Qt::AlignCenter);
		labelAutoCalibrate->setVisible(false);

		// toggle button
		btnAutoCalibrate = new QPushButton("Checkerboard", this);
		btnAutoCalibrate->setObjectName("secondary");
		btnAutoCalibrate->setCursor(Qt::PointingHandCursor);
		btnAutoCalibrate->setFixedWidth(150);
		btnAutoCalibrate->setVisible(false);
		
		layout->addWidget(labelAutoCalibrate, 2, 0);
		layout->addWidget(btnAutoCalibrate, 3, 0, Qt::AlignCenter);

		connect(btnAutoCalibrate, &QPushButton::clicked, this, &CameraModeConfig::toggleAutoCalibrate);
	}


	void CameraModeConfig::toggleAutoCalibrate() {

		calibrationMode = "auto";
		isActiveCalibrate = !isActiveCalibrate;

		labelAutoCalibrate->setVisible(!isActiveCalibrate);
		btnAutoCalibrate->setVisible(!isActiveCalibrate);
		labelManualCalibrate->setVisible(!isActiveCalibrate);
		btnManualCalibrate->setVisible(!isActiveCalibrate);
		
		btnCalibrate->setText(isActiveCalibrate ? "Cancel" : "Close");

		isActiveCalibrate ? emit autoCalibrateStarted() :  emit autoCalibrateStopped();
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	MANUAL CALIBRATE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void CameraModeConfig::initManualCalibrate() {

		// label
		labelManualCalibrate = new QLabel("Place corner points manually", this);
		labelManualCalibrate->setObjectName("small");
		labelManualCalibrate->setAlignment(Qt::AlignCenter);
		labelManualCalibrate->setVisible(false);

		// toggle button
		btnManualCalibrate = new QPushButton("Manual", this);
		btnManualCalibrate->setObjectName("secondary");
		btnManualCalibrate->setCursor(Qt::PointingHandCursor);
		btnManualCalibrate->setFixedWidth(150);
		btnManualCalibrate->setVisible(false);
		
		layout->addWidget(labelManualCalibrate, 2, 1);
		layout->addWidget(btnManualCalibrate, 3, 1, Qt::AlignCenter);

		connect(btnManualCalibrate, &QPushButton::clicked, this, &CameraModeConfig::toggleManualCalibrate);
	}

	
	void CameraModeConfig::toggleManualCalibrate() {

		calibrationMode = "manual";
		isActiveCalibrate = !isActiveCalibrate;

		labelAutoCalibrate->setVisible(!isActiveCalibrate);
		btnAutoCalibrate->setVisible(!isActiveCalibrate);
		labelManualCalibrate->setVisible(!isActiveCalibrate);
		btnManualCalibrate->setVisible(!isActiveCalibrate);
		
		btnCalibrate->setText(isActiveCalibrate ? "Cancel" : "Close");

		btnCalibrateReset->setVisible(isActiveCalibrate);
		btnCalibrateSave->setVisible(isActiveCalibrate);

		isActiveCalibrate ? emit manualCalibrateStarted() :  emit manualCalibrateStopped();
	}


	void CameraModeConfig::resetManualCalibrate() {

		emit manualCalibrateReset();
	}




/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PAPERSCOPE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void CameraModeConfig::initPaperScope() {

		connect(paperScope, &PaperScope::trackingModeChanged, this, &CameraModeConfig::onTrackingModeChanged);
	}


	void CameraModeConfig::onTrackingModeChanged(PSTrackingMode newMode, PSTrackingMode oldMode) {

		if(oldMode == PSTrackingMode::Calibrate && btnCalibrate->isChecked()) {

			toggleCalibrate();
		}
		else if(oldMode == PSTrackingMode::None) {

			btnCalibrate->setVisible(true);
		}
	}
