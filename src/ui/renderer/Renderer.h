/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#pragma once

	// Qt
	#include <QWidget>
	#include <QGraphicsView>
	#include <QGraphicsScene>
	#include <QGraphicsPixmapItem>
	#include <QGraphicsRectItem>
	#include <QTabBar>
	#include <QVBoxLayout>
	#include <QLabel>

	// OpenCV
	#include <opencv2/opencv.hpp>

	// App
	#include "RenderMode.h"
	#include "DraggableRectItem.h"
	#include "PreferencesTab.h"
	#include "mode/CameraModeConfig.h"
	#include "mode/PaperScopeModeConfig.h"
    #include "../../paperscope/PaperScope.h"



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CLASS DECLARATION
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


class Renderer : public QWidget {

	Q_OBJECT

	public:
		
		explicit Renderer(QWidget *parent = nullptr);
		~Renderer();


	private:

		// ui
        void initUserInterface();
		QVBoxLayout *layout;
		QTabBar *tabBar;

		// renderer
		void initRenderer();
		void updateRenderer(cv::Mat &mat, std::vector<cv::Point2f> points);
		RenderMode renderMode;
		QGraphicsView renderView;
		QGraphicsScene renderScene;
		QGraphicsPixmapItem renderPixmapItem;
		float scaling;

		// camera
		void initCamera();
		CameraModeConfig *cameraModeConfig;
		
		// calibrate
		QString calibrationMode;
		DraggableRectItem *cornerPointTL;
		DraggableRectItem *cornerPointTR;
		DraggableRectItem *cornerPointBR;
		DraggableRectItem *cornerPointBL;
		bool isCalibrating;
		
		// paperscope
		void initPaperScope();
		PaperScopeModeConfig *paperScopeModeConfig;
		PaperScope *paperScope;

		// preferences
		void initPreferences();
		PreferencesTab *preferencesTab;

		// settings
		void initSettings();


	public slots:

		// ui
		void onTabChanged(int index);

		// calibrate
		void startManualCalibrate();
		void saveManualCalibrate();
		void resetManualCalibrate();
		void stopManualCalibrate();

		// processing
		void update(cv::Mat mat, std::vector<cv::Point2f> points);

		// settings
		void onSettingsUpdated(QString key, QVariant value);
};

