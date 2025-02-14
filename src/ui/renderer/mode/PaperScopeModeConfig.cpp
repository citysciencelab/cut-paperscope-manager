/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#include "PaperScopeModeConfig.h"

	// Qt
	#include <QLabel>



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONSTRUCTOR
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	PaperScopeModeConfig::PaperScopeModeConfig(PaperScope *paperScope, QWidget *parent)
		: QWidget(parent),
		  layout(nullptr),
		  paperScope(paperScope),
		  selectPaperScopeView(nullptr),
		  labelThresholdDark(nullptr),
		  sliderThresholdDark(nullptr),
		  labelThresholdLight(nullptr),
		  sliderThresholdLight(nullptr),
		  labelRed(nullptr),
		  sliderRed(nullptr),
		  labelSmoothing(nullptr),
		  sliderSmoothing(nullptr),
		
		  btnDataset(nullptr)
	{

		// init member
		initUserInterface();
		initPaperScopeView();
		initThreshold();
		initRed();
		initSmoothing();
		initDataset();
	}


	PaperScopeModeConfig::~PaperScopeModeConfig() {
		
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	USER INTERFACE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PaperScopeModeConfig::initUserInterface() {

		// layout
		layout = new QGridLayout();
		layout->setContentsMargins(25,25,25,0);
		layout->setSpacing(10);
		layout->setColumnMinimumWidth(1, 30);
		setLayout(layout);
	}


/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	VIEW MODE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PaperScopeModeConfig::initPaperScopeView() {

		selectPaperScopeView = new QComboBox(this);
		selectPaperScopeView->setObjectName("secondary");
		selectPaperScopeView->setCursor(Qt::PointingHandCursor);
		selectPaperScopeView->addItem("2D Plane");
		selectPaperScopeView->addItem("Processing");
		selectPaperScopeView->addItem("Threshold");
		selectPaperScopeView->addItem("Streets");
		selectPaperScopeView->addItem("Bounding Boxes");
		selectPaperScopeView->addItem("Contours");
		selectPaperScopeView->setCurrentIndex(2);

		layout->addWidget(selectPaperScopeView, 0, 1, Qt::AlignRight);

		connect(selectPaperScopeView, &QComboBox::currentIndexChanged, this, &PaperScopeModeConfig::onPaperScopeViewChanged);
	}	


	void PaperScopeModeConfig::onPaperScopeViewChanged(int index) {
		
		Settings::instance()->saveInt("paperscope_viewmode", index);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	THRESHOLD
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PaperScopeModeConfig::initThreshold() {

		// dark
		labelThresholdDark = new QLabel("Threshold Dark", this);
		labelThresholdDark->setObjectName("small");
		
		sliderThresholdDark = new QSlider(Qt::Horizontal, this);
		sliderThresholdDark->setMinimum(0);
		sliderThresholdDark->setMaximum(255);
		sliderThresholdDark->setValue(Settings::instance()->getInt("threshold_dark", 50));

		layout->addWidget(labelThresholdDark, 1, 0, Qt::AlignVCenter);
		layout->addWidget(sliderThresholdDark, 2, 0);

		connect(sliderThresholdDark, &QSlider::valueChanged, this, &PaperScopeModeConfig::onThresholdDarkChanged);

		// light
		labelThresholdLight = new QLabel("Threshold Light", this);
		labelThresholdLight->setObjectName("small");
		
		sliderThresholdLight = new QSlider(Qt::Horizontal, this);
		sliderThresholdLight->setMinimum(0);
		sliderThresholdLight->setMaximum(255);
		sliderThresholdLight->setValue(Settings::instance()->getInt("threshold_light", 180));

		layout->addWidget(labelThresholdLight, 1, 1);
		layout->addWidget(sliderThresholdLight, 2, 1);

		connect(sliderThresholdLight, &QSlider::valueChanged, this, &PaperScopeModeConfig::onThresholdLightChanged);
	}


	void PaperScopeModeConfig::onThresholdDarkChanged(int value) {
		
		Settings::instance()->saveInt("threshold_dark", value);
	}


	void PaperScopeModeConfig::onThresholdLightChanged(int value) {
		
		Settings::instance()->saveInt("threshold_light", value);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	RED
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PaperScopeModeConfig::initRed() {

		labelRed = new QLabel("Red Threshold", this);
		labelRed->setObjectName("small");

		sliderRed = new QSlider(Qt::Horizontal, this);
		sliderRed->setMinimum(0);
		sliderRed->setMaximum(255);
		sliderRed->setValue(Settings::instance()->getInt("threshold_red", 150));

		layout->addWidget(labelRed, 3, 0);
		layout->addWidget(sliderRed, 4, 0);

		connect(sliderRed, &QSlider::valueChanged, this, &PaperScopeModeConfig::onRedChanged);
	}


    void PaperScopeModeConfig::onRedChanged(int value) {
		
		Settings::instance()->saveInt("threshold_red", value);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SMOOTHING
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PaperScopeModeConfig::initSmoothing() {

		labelSmoothing = new QLabel("Smoothing", this);
		labelSmoothing->setObjectName("small");

		sliderSmoothing = new QSlider(Qt::Horizontal, this);
		sliderSmoothing->setMinimum(0);
		sliderSmoothing->setMaximum(100);
		sliderSmoothing->setValue(Settings::instance()->getFloat("smoothing", 0.7) * 100);

		layout->addWidget(labelSmoothing, 3, 1);
		layout->addWidget(sliderSmoothing, 4, 1);

		connect(sliderSmoothing, &QSlider::valueChanged, this, &PaperScopeModeConfig::onSmoothingChanged);
	}


	void PaperScopeModeConfig::onSmoothingChanged(int value) {
		
		Settings::instance()->saveFloat("smoothing", value / 100.0);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DATASET
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PaperScopeModeConfig::initDataset() {

		//btnDataset = new QPushButton("Capture Dataset", this);
		//btnDataset->setObjectName("small");

		//layout->addWidget(btnDataset, 3, 0, 1, 2);

		//connect(btnDataset, &QPushButton::clicked, this, &PaperScopeModeConfig::onDatasetClicked);
	}


	void PaperScopeModeConfig::onDatasetClicked() {
		
		Settings::instance()->saveBool("capture_dataset", true);
	}
