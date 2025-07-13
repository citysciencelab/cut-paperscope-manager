/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#include "PreferencesTab.h"

	// App
	#include "../../MainWindow.h"
	#include "../../global/Settings.h"



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONSTRUCTOR
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	PreferencesTab::PreferencesTab(QWidget *parent)
		: QWidget(parent),
		  layout(nullptr),
		  buttonLayout(nullptr),
		  buttonReset(nullptr),
		  buttonSave(nullptr),
		  labelServerUrl(nullptr),
		  inputServerUrl(nullptr),
		  labelWebsocketPort(nullptr),
		  inputWebsocketPort(nullptr),
		  labelPaperscopeScaling(nullptr),
		  inputPaperscopeScaling(nullptr)
	{

		// init member
		initUserInterface();
		initServerInput();
		initPaperscopeInput();
		initButtons();
	}


	PreferencesTab::~PreferencesTab() {

	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	USER INTERFACE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PreferencesTab::initUserInterface() {

		// layout
		layout = new QGridLayout();
		layout->setContentsMargins(50,50,50,0);
		layout->setSpacing(10);
		layout->setColumnMinimumWidth(1, 15);
		layout->setRowMinimumHeight(1, 25);
		setLayout(layout);
	}


	void PreferencesTab::initButtons() {

		// button layout
		buttonLayout = new QHBoxLayout();

		// reset button
		buttonReset = new QPushButton("Reset");
		buttonReset->setFixedWidth(130);
        buttonReset->setObjectName("outline");
        buttonReset->setCursor(Qt::PointingHandCursor);
        buttonLayout->addWidget(buttonReset);
		connect(buttonReset, &QPushButton::clicked, this, &PreferencesTab::reset);

		// save button
		buttonSave = new QPushButton("Save");
		buttonSave->setFixedWidth(170);
        buttonSave->setCursor(Qt::PointingHandCursor);
        buttonLayout->addWidget(buttonSave);
		connect(buttonSave, &QPushButton::clicked, this, &PreferencesTab::save);

		layout->addLayout(buttonLayout, 4, 2);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SERVER INPUT
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PreferencesTab::initServerInput() {

		// label server url
		labelServerUrl = new QLabel("Server URL");
		layout->addWidget(labelServerUrl, 0, 0);

		// input server url
		inputServerUrl = new QLineEdit();
		inputServerUrl->setText(Settings::instance()->getString("server_url","https://paperscope.comodeling.city/"));
		layout->addWidget(inputServerUrl, 1, 0);

		// label websocket port
		labelWebsocketPort = new QLabel("Websocket Port");
		layout->addWidget(labelWebsocketPort, 0, 2);

		// input websocket port
		inputWebsocketPort = new QLineEdit();
		inputWebsocketPort->setText(Settings::instance()->getString("websocket_port", "443"));
		layout->addWidget(inputWebsocketPort, 1, 2);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PAPERSCOPE INPUT
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PreferencesTab::initPaperscopeInput() {

		// label scaling
		labelPaperscopeScaling = new QLabel("PaperScope Scaling");
		layout->addWidget(labelPaperscopeScaling, 2, 0);

		// input scaling
		inputPaperscopeScaling = new QLineEdit();
		inputPaperscopeScaling->setText(QString::number(Settings::instance()->getFloat("scaling", 1.0)));
		layout->addWidget(inputPaperscopeScaling, 3, 0);
	}

 

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PREFERENCES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PreferencesTab::save() {

		// validate input
		QString serverUrl = inputServerUrl->text();
		if(!serverUrl.endsWith("/")) { serverUrl.append("/"); }
		QString scaling = inputPaperscopeScaling->text().replace(",", ".");
		inputPaperscopeScaling->setText(scaling);

		// save settings
		Settings::instance()->saveString("server_url", serverUrl);
		Settings::instance()->saveString("websocket_port", inputWebsocketPort->text());
		Settings::instance()->saveFloat("scaling", scaling.toFloat());
		Settings::instance()->save();
	}


	void PreferencesTab::reset() {

		inputServerUrl->setText(Settings::instance()->saveString("server_url", "https://paperscope.comodeling.city/"));
		inputWebsocketPort->setText(Settings::instance()->saveString("websocket_port", "443"));
		inputPaperscopeScaling->setText(QString::number(Settings::instance()->saveFloat("scaling", 1.0)));

		save();
	}
