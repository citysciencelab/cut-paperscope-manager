/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#pragma once
	
	// Qt
	#include <QWidget>
	#include <QGridLayout>
	#include <QPushButton>
	#include <QLabel>
	#include <QComboBox>

	// App
	#include "../../../paperscope/PaperScope.h"



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CLASS DECLARATION
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


class CameraModeConfig : public QWidget {
	
	Q_OBJECT
	
	public:
		
		explicit CameraModeConfig(PaperScope *paperScope, QWidget *parent = nullptr);
		~CameraModeConfig();


	private:

		// ui
		void initUserInterface();
		QGridLayout *layout;

		// config
		void initCalibrationMode();
		QLabel *labelCalibrationMode;
		QComboBox *selectCalibrationMode;

		// calibrate
		void initCalibrate();
		void toggleCalibrate();
		bool isCalibrate;
		bool isActiveCalibrate;
		QString calibrationMode;
		QPushButton *btnCalibrate;
		QPushButton *btnCalibrateSave;
		QPushButton *btnCalibrateReset;

		// auto calibrate
		void initAutoCalibrate();
		QLabel *labelAutoCalibrate;
		QPushButton *btnAutoCalibrate;

		// manual calibrate
		void initManualCalibrate();
		QLabel *labelManualCalibrate;
		QPushButton *btnManualCalibrate;

		// paperscope
		void initPaperScope();
		PaperScope *paperScope;


	signals:

		// calibrate
		void autoCalibrateStarted();
		void autoCalibrateStopped();
		void manualCalibrateStarted();
		void manualCalibrateSaved();
		void manualCalibrateReset();
		void manualCalibrateStopped();


	public slots:

		// config
		void changeCalibrationMode(int index);

		// calibrate
		void saveCalibrate();
		void toggleAutoCalibrate();
		void toggleManualCalibrate();
		void resetManualCalibrate();

		// paperscope
		void onTrackingModeChanged(PSTrackingMode newMode, PSTrackingMode oldMode);
};

