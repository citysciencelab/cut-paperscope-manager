/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#pragma once

	// Qt
	#include <QObject>
	#include <QJsonObject>

	// OpenCV
	#include <opencv2/opencv.hpp>

	// App
	#include "../PSTrackingMode.h"
	#include "../PSViewMode.h"
	#include "../detect/PSCandidate.h"
	#include "../detect/PSShapeType.h"
	#include "PSObject.h"
	#include "../../global/Settings.h"
	#include "../../ui/renderer/RenderMode.h"



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CLASS DECLARATION
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


class PSDescribe : public QObject {
	
	Q_OBJECT

	public:
		
		explicit PSDescribe(QObject *parent = nullptr);
		~PSDescribe();

		// processing
		void init();
		void update(cv::Mat *mTracking, cv::Mat *mRender, PSTrackingMode trackingMode, std::vector<PSCandidate> &candidates);
		void close();

		// opencv
		cv::Mat *matTracking;
		cv::Mat *matRender;

		// server
		void onRequestSent();


	private:

		// scene
		void updateScene(std::vector<PSCandidate> &candidates);
		std::vector<PSObject> objects;
		int countValidObjects;
		int minConfidence;

		// server
		void sendRequest();
		bool needsRequest;
		qint64 timestampSent;
		bool isSending;
		QString projectId;

		// renderer
		void drawScene(std::vector<PSCandidate> &candidates);
		RenderMode renderMode;
        PSViewMode viewMode;


	signals:

		void sceneUpdated(QString url, QJsonObject json, std::function<void(QJsonObject data)> callback);

	
	public slots:

		// settings
		void onSettingsUpdated(QString key, QVariant value);
		
};

