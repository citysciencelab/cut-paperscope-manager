/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#pragma once
	
	// Qt
	#include <QWidget>
	#include <QLabel>
	#include <QGridLayout>
	#include <QComboBox>
	#include <QSlider>
    #include <QPushButton>

	// App
	#include "../../../paperscope/PaperScope.h"



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CLASS DECLARATION
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


class PaperScopeModeConfig : public QWidget {
	
	Q_OBJECT
	
	public:
		
		explicit PaperScopeModeConfig(PaperScope *paperScope, QWidget *parent = nullptr);
		~PaperScopeModeConfig();


	private:

		// ui
		void initUserInterface();
		QGridLayout *layout;

		// paperscope
		void initPaperScope();
		PaperScope *paperScope;

		// config
		void initPaperScopeView();
		void initThreshold();
		void initRed();
		void initSmoothing();
		void initDataset();
		QComboBox *selectPaperScopeView;
		QLabel *labelThresholdDark;
		QSlider *sliderThresholdDark;
		QLabel *labelThresholdLight;
		QSlider *sliderThresholdLight;
		QLabel *labelRed;
		QSlider *sliderRed;
		QLabel *labelSmoothing;
		QSlider *sliderSmoothing;

		// dataset
		QPushButton *btnDataset;

	
	public slots:

		// config
		void onPaperScopeViewChanged(int index);
		void onThresholdDarkChanged(int value);
		void onThresholdLightChanged(int value);
		void onRedChanged(int value);
		void onSmoothingChanged(int value);
		void onDatasetClicked();
};

