/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#pragma once

	// Qt
	#include <QWidget>
	#include <QVBoxLayout>
	#include <QSvgWidget>
	#include <QLabel>
	#include <QLineEdit>
	#include <QComboBox>
	#include <QPushButton>
	#include <QCameraDevice>

	// App
    #include "../../paperscope/PaperScope.h"
    #include "../../paperscope/PSTrackingMode.h"
	#include "../../global/Settings.h"




/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CLASS DECLARATION
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


class MainNavi : public QWidget {

	Q_OBJECT

	public:
	
		explicit MainNavi(QWidget *parent = nullptr);
		~MainNavi();


	private:

		// layout
		void initLayout();
		QVBoxLayout *layout;

		// header
		void initHeader();
		QSvgWidget *logo;

		// project
		void initProject();
		void loadProject(QString projectId);
		QLabel *labelProject;
		QLineEdit *inputProject;
		QLabel *labelSocketStatus;

		// camera
		void initCamera();
		QList<QCameraDevice> cameraDevices;
		QLabel *labelCameraDevice;
		QComboBox *selectCameraDevice;
		QComboBox *selectCameraFormat;
		QPushButton *btnCamera;
		QSvgWidget *separator;

		// paperscope
		void initPaperScope();
		PaperScope *paperScope;
		QLabel *labelPaperScope;
		QPushButton *btnRealtime;
		QPushButton *btnSnapshot;

		// settings
		void initSettings();


	public slots:

		// project
		void onProjectInputChanged();
		void onProjectInputSubmit();
		void onProjectError(QString message, int code, QString url);

		// camera
		void initCameraDevices(QList<QCameraDevice> cameras);
		void setCameraFormats(const QCameraDevice camera);
		void onCameraDeviceSelected(int index);
		void onCameraFormatSelected(int index);

		// paperscope
		void onBtnRealtimeClicked();
		void onBtnSnapshotClicked();
		void onTrackingModeChanged(PSTrackingMode newMode, PSTrackingMode oldMode);

		// settings
		void onSettingsUpdated(QString key, QVariant value);
};


