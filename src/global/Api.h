/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#pragma once

	// Qt
	#include <QObject>
	#include <QNetworkAccessManager>
	#include <QNetworkReply>
	#include <QJsonObject>

	// App
	#include "Settings.h"



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CLASS DECLARATION
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


class Api : public QObject {
	
	Q_OBJECT

	public:

		static Api* instance();
		~Api();

		// getters
		QString getBaseUrl();


	private:

		// singleton
		static bool instanceFlag;
		static Api* apiInstance;
		Api();

		// manager
		void initManager();
		QString baseUrl;
		QNetworkAccessManager *networkManager;
		
		// error
		void onError(QNetworkReply *reply);

		// settings
		void initSettings();


	signals:

		// signals
        void error(QString message, int code, QString url);


	public slots:

		// get requests
		void get(QString url, std::function<void(QJsonObject data)> callback);
		
		// post requests
		void postResponse(QString url, QJsonObject json, std::function<void(QJsonObject data)> callback);
		void post(QString url, QJsonObject json, std::function<void(QJsonObject data)> callback);

		// settings
		void onSettingsUpdated(QString key, QVariant value);
};

