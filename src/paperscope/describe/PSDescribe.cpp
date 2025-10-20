/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#include "PSDescribe.h"

	// Qt
	#include <QJsonArray>
	#include <QDebug>
	#include <QTimer>
	#include <QJsonDocument>
	#include <QJsonObject>
	#include <QDateTime>

	// App
	#include "../../global/Settings.h"
	#include "../../global/Api.h"



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONSTRUCTOR
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	PSDescribe::PSDescribe(QObject *parent)
		: QObject(parent),
		  matTracking(nullptr),
		  matRender(nullptr)
	{

		// init properties
		renderMode = RenderMode::Camera,
		viewMode = PSViewMode::Threshold;
		minConfidence = 70;
		needsRequest = false;
		isSending = false;
		timestampSent = QDateTime::currentSecsSinceEpoch();

		// init member
		connect(this, &PSDescribe::sceneUpdated, Api::instance(), &Api::post);
	}


	PSDescribe::~PSDescribe() {

	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PROCESSING
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSDescribe::init() {

	}


	void PSDescribe::update(cv::Mat *mTracking, cv::Mat *mRender, PSTrackingMode trackingMode, std::vector<PSCandidate> &candidates) {

		matTracking = mTracking;
		matRender = mRender;

		// skip loop
		if(trackingMode != PSTrackingMode::Tracking || matTracking->empty()) {
			if(!objects.empty()) { objects.clear(); }
			return;
		}
		
		updateScene(candidates);
		drawScene(candidates);
	}


	void PSDescribe::close() {

	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SCENE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSDescribe::updateScene(std::vector<PSCandidate> &candidates) {

		// reduce confidence of existing objects to filter out old objects
		for(PSObject &object : objects) { object.confidence -= 5; }

		// iterate all candidates
		for(PSCandidate &candidate : candidates) {

			// check if object already exists
			bool found = false;
			for(PSObject &object : objects) {
				if(object.isSame(candidate.getPoints(), candidate.shapeType)) {
					found = true;
					break;
				}
			}

			// create object from candidate
			if(!found) {
				PSObject object(candidate.getPoints(), candidate.shapeType, matTracking, matRender);
				objects.push_back(object);
			}
		}

		// remove old objects from scene
		std::vector<PSObject> validObjects;
		int valid = 0;
		for(int i = 0; i < (int) objects.size(); i++) {
			if(objects[i].confidence > 0) { validObjects.push_back(objects[i]); }
			if(objects[i].confidence > minConfidence) { valid++; }
		}
		objects = validObjects;

		// check if request is needed
		if(countValidObjects != valid) { needsRequest = true; }

		countValidObjects = valid;
		sendRequest();
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SERVER
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSDescribe::sendRequest() {

		if(isSending || !needsRequest || projectId.isEmpty()) { return; }

		// save timestamp of request
		qint64 timestamp = QDateTime::currentSecsSinceEpoch();
		if(timestamp - timestampSent < 2) { return; }
		timestampSent = timestamp;

		QJsonObject data;
		QJsonArray scene;


		// convert objects to form data
		for(PSObject &object : objects) {

			if(object.confidence <= minConfidence) { continue; }

			// default properties
			QJsonObject obj;
			obj["uid"] = object.uid.c_str();
			obj["shape"] = (int) object.shapeType;
			obj["color"] = (int) object.colorIndex;

			// points data
			QJsonArray points;
			for(cv::Point2f &point : object.points) {
				QJsonObject p;
				p["x"] = point.x;
				p["y"] = point.y;
				points.append(p);
			}
			obj["points"] = points;
			
			scene.append(obj);
		}
		data.insert("slug", projectId);
		data["scene"] = scene;

		// trigger api call on main thread via signal
		isSending = true;
		needsRequest = false;
		emit sceneUpdated("api/project/scene/save",  data, std::bind(&PSDescribe::onRequestSent, this));
	}


	void PSDescribe::onRequestSent() {

		qDebug() << "scene saved";
		isSending = false;
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DRAW
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSDescribe::drawScene(std::vector<PSCandidate> &candidates) {
		
		if(renderMode != RenderMode::PaperScope || viewMode != PSViewMode::Contours) { return; }

		cv::multiply(*matRender, 0.5, *matRender);

		// draw objects
		for(int i = 0; i < (int) objects.size(); i++) {
			
            objects[i].drawContour(minConfidence);

			// draw confidence above object
            cv::Rect boundingRect = cv::boundingRect(objects[i].candidatePoints);
            if(boundingRect.width < 1 || boundingRect.height < 1) { continue; }
            cv::putText(*matRender, std::to_string(objects[i].confidence), cv::Point(boundingRect.x, boundingRect.y - 8), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
		}

		// show candidate and object count in matRender
        cv::putText(*matRender, "Candidates: " + std::to_string(candidates.size()), cv::Point(30, 80), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
        cv::putText(*matRender, "Objects: " + std::to_string(countValidObjects), cv::Point(30, 110), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SETTINGS
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSDescribe::onSettingsUpdated(QString key, QVariant value) {

		if(key == "renderMode") {
			renderMode = (RenderMode) value.toInt();
		}
		else if(key == "paperscope_viewmode") {
			viewMode = (PSViewMode) value.toInt();
		}
		else if(key == "project_id") {
			projectId = value.toString();
		}
	}

