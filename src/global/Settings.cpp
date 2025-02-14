/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#include "Settings.h"

	// Qt
	#include <QSettings>
	#include <QDir>
	#include <QStandardPaths>
	#include <QPointF>



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SINGLETON
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	// static properties
	bool Settings::instanceFlag = false;
	Settings* Settings::settingsInstance = nullptr;


	Settings* Settings::instance() {

		if (!instanceFlag) {
			settingsInstance = new Settings();
			instanceFlag = true;
		} 

		return settingsInstance;
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONSTRUCTOR
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	Settings::Settings() {

		load();
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	GETTER
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	QString Settings::getString(QString key, QString defaultValue) {

		if(unsavedSettings.contains(key)) { return unsavedSettings[key].toString(); }
		return defaultValue;
	}


	int Settings::getInt(QString key, int defaultValue) {

		if(unsavedSettings.contains(key)) { return unsavedSettings[key].toInt(); }
		return defaultValue;
	}


	float Settings::getFloat(QString key, float defaultValue) {

		if(unsavedSettings.contains(key)) { return unsavedSettings[key].toFloat(); }
		return defaultValue;
	}


	bool Settings::getBool(QString key, bool defaultValue) {

		if(unsavedSettings.contains(key)) { return unsavedSettings[key].toBool(); }
		return defaultValue;
	}


	cv::Mat Settings::getMat(QString key, cv::Mat defaultValue) {

		if(unsavedSettings.contains(key)) { 
			
			cv::FileStorage file(unsavedSettings[key].toString().toStdString(), cv::FileStorage::READ);
			cv::Mat mat;
			file["matName"] >> mat;
			file.release();
			return mat;
		}

		return defaultValue;
	}


	QJsonObject Settings::getJsonObject(QString key, QJsonObject defaultValue) {

		if(unsavedSettings.contains(key)) { return unsavedSettings[key].toJsonObject(); }
		return defaultValue;
	}


	std::vector<cv::Point2f> Settings::getPoints(QString key, std::vector<cv::Point2f> defaultValue) {

		if(unsavedSettings.contains(key)) {

			QList<QVariant> list = unsavedSettings[key].toList();
			std::vector<cv::Point2f> points;
			for(int i = 0; i < list.size(); i++) {
				QPointF point = list[i].toPointF();
				points.push_back(cv::Point2f(point.x(), point.y()));
			}

			return points;
		}

		return defaultValue;
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SAVE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	QString Settings::saveString(QString key, QString value) {

		unsavedSettings[key] = value;
		emit settingsUpdated(key, QVariant(value));
		return value;
	}


	int Settings::saveInt(QString key, int value) {

		unsavedSettings[key] = value;
		emit settingsUpdated(key, QVariant(value));
		return value;
	}


	float Settings::saveFloat(QString key, float value) {

		unsavedSettings[key] = value;
		emit settingsUpdated(key, QVariant(value));
		return value;
	}


	bool Settings::saveBool(QString key, bool value) {

		unsavedSettings[key] = value;
		emit settingsUpdated(key, QVariant(value));
		return value;
	}


	cv::Mat Settings::saveMat(QString key, cv::Mat value) {

		// create path
		QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
		QDir dir(path);
		if(!dir.exists()) { dir.mkpath("."); }

		// write file
		std::string filename = path.toStdString()+"/"+key.toStdString()+".yaml";
		cv::FileStorage file(filename, cv::FileStorage::WRITE);
		file << "matName" << value;
		file.release();

		// save key
		unsavedSettings[key] = QString::fromStdString(filename);
		emit settingsUpdated(key, QVariant(QString::fromStdString(filename)));

		return value;
	}


	QJsonObject Settings::saveJsonObject(QString key, QJsonObject value) {

		unsavedSettings[key] = value;
		emit settingsUpdated(key, QVariant(value));
		return value;
	}


	std::vector<cv::Point2f> Settings::savePoints(QString key, std::vector<cv::Point2f> value) {

		// prepare list
		QList<QVariant> list;
		for(int i = 0; i < value.size(); i++) { list.append(QPointF(value[i].x, value[i].y)); }

		// save key
		unsavedSettings[key] = list;
		emit settingsUpdated(key, QVariant(list));

		return value;
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SAVE / LOAD
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void Settings::save() {

		QSettings settings;
		QMap<QString, QVariant>::iterator i;
		for (i = unsavedSettings.begin(); i != unsavedSettings.end(); ++i) {
			settings.setValue(i.key(), i.value());
		}
	}


	void Settings::load() {

		QSettings settings;
		QStringList keys = settings.allKeys();
		for(int i = 0; i < keys.size(); i++) {
			unsavedSettings[keys[i]] = settings.value(keys[i]);
		}
	}