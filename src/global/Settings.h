/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#pragma once

	// Qt
	#include <QObject>
	#include <QMap>
	#include <QVariant>
	#include <QJsonObject>

	// OpenCV
	#include <opencv2/opencv.hpp>



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CLASS DECLARATION
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


class Settings : public QObject {
	
	Q_OBJECT

	public:

		static Settings* instance();
		~Settings() { instanceFlag = false; };

		// getter
		QString getString(QString key, QString defaultValue = "");
		int getInt(QString key, int defaultValue = 0);
		float getFloat(QString key, float defaultValue = 0.0);
		bool getBool(QString key, bool defaultValue = false);
		cv::Mat getMat(QString key, cv::Mat defaultValue = cv::Mat());
		QJsonObject getJsonObject(QString key, QJsonObject defaultValue = QJsonObject());
		std::vector<cv::Point2f> getPoints(QString key, std::vector<cv::Point2f> defaultValue = std::vector<cv::Point2f>());

		// save
		QString saveString(QString key, QString value);
		int saveInt(QString key, int value);
		float saveFloat(QString key, float value);
		bool saveBool(QString key, bool value);
		cv::Mat saveMat(QString key, cv::Mat value);
		QJsonObject saveJsonObject(QString key, QJsonObject value);
		std::vector<cv::Point2f> savePoints(QString key, std::vector<cv::Point2f> value);


	private:
	
		// singleton
		static bool instanceFlag;
		static Settings* settingsInstance;
		Settings();

		// changes
		QMap<QString, QVariant> unsavedSettings;


	signals:

		void settingsUpdated(QString key, QVariant value);


	public slots:

		void save();
		void load();
};

