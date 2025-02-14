/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#include "Broadcast.h"

	// Qt
	#include <QUrl>
	#include <QNetworkRequest>
	#include <QNetworkReply>
	#include <QJsonDocument>
	#include <QJsonParseError>
	#include <QJsonArray>
	#include <QByteArray>
	#include <QTimer>

	// App
	#include "Settings.h"
	#include "Api.h"



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SINGLETON
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	// static properties
	bool Broadcast::instanceFlag = false;
	Broadcast* Broadcast::broadcastInstance = nullptr;


	Broadcast* Broadcast::instance() {

		if (!instanceFlag) {
			broadcastInstance = new Broadcast();
			instanceFlag = true;
		} 

		return broadcastInstance;
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONSTRUCTOR
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	Broadcast::Broadcast()
		: QObject(nullptr),
		  webSocket(nullptr)
	{
		
		// init properties
		webSocketAppId = QString("cxu73Avj8Kny2gpEQeLqD4fXTVFPhzMR");
		webSocketPort = QString("443");

		// init member
		connectWebSocket();	
	}


	Broadcast::~Broadcast() {

		instanceFlag = false;

		if(webSocket) { delete webSocket; }
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	WEBSOCKET
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void Broadcast::connectWebSocket() {

		// create url
		QString server = Api::instance()->getBaseUrl().split("/").at(2);
		QUrl url = QUrl("wss://"+server+"/app/"+webSocketAppId+"?protocol=7&client=js&version=8.4.0-rc2&flash=false");

		// init websocket
		if(!webSocket) { 
			webSocket = new QWebSocket(); 
			connect(webSocket, &QWebSocket::connected, this, &Broadcast::onWebSocketConnected);
   	 		connect(webSocket, &QWebSocket::disconnected, this, &Broadcast::onWebSocketClosed);
			connect(webSocket, &QWebSocket::errorOccurred, this, &Broadcast::onWebSocketError);
			connect(webSocket, &QWebSocket::textMessageReceived, this, &Broadcast::onMessage);
		} 
		else { 
			webSocket->close(); 
		}
		
		webSocket->open(url);
	}


	// emit connected signal after websocket id is available (see onMessage)
	void Broadcast::onWebSocketConnected() {

		Settings::instance()->saveBool("websocket", true);
		QTimer::singleShot(30*1000, this, &Broadcast::sendPing);
	}


	void Broadcast::onWebSocketClosed() {

		webSocketId = "";
		webSocketAuth.clear();

		qDebug() << "Websocket closed";

		Settings::instance()->saveBool("websocket", false);
		emit closed();
	}


	void Broadcast::onWebSocketError(QAbstractSocket::SocketError error) {

		qDebug() << "Websocket error: " << error;
	}


	void Broadcast::sendPing() {

		if(!webSocketId.isEmpty()) {

			QJsonObject data;
			data["event"] = "pusher:ping";
			data["data"] = QJsonArray();
			sendMessage(data);
			QTimer::singleShot(30*1000, this, &Broadcast::sendPing);
		}
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	MESSAGE
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void Broadcast::onMessage(QString message) {

		// initial response for socket id
		if(message.contains("pusher:connection_established")) {
			
			// get socket id
			QRegularExpression re("socket_id[^\\d]*([\\d.]+)");
			QRegularExpressionMatch match = re.match(message);
			webSocketId = match.captured(1);

			emit connected();
		}
		// default pusher channel message
		else if(message.contains("\"channel\"") && !message.contains("pusher_internal")) {

			onChannelMessage(message);
		}
		else if(message.contains("pusher_internal")) {
			// do nothing
		}
		else if(message.contains("pusher:pong")) {
			// do nothing
		}
		else {
			
			qDebug() << "Unknown Websocket message: " << message;
		}	

		emit messageReceived(message);
	}


	void Broadcast::sendMessage(QString message) {

		if(webSocket) {
			webSocket->sendTextMessage(message);
		}
	}


	void Broadcast::sendMessage(QJsonObject data) {

		QJsonDocument doc(data);
		sendMessage(doc.toJson(QJsonDocument::Compact));
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PUSHER
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void Broadcast::authPusherChannel(QString channel, std::function<void(QString auth)> callback) {

		if(webSocketId.isEmpty()) { 
			qDebug() << "unable to authenticate pusher channel. Websocket id is empty";
			return; 
		}

		// post data
		QJsonObject data = QJsonObject();
		data["socket_id"] = webSocketId;
		data["channel_name"] = channel;
		
		// send request
		Api::instance()->postResponse("api/broadcasting/auth", data, [=](QJsonObject response) { 
			QString auth = response.value("auth").toString();
			webSocketAuth.insert(channel,auth);
			callback(auth);
		});
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	PUSHER CHANNELS
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void Broadcast::subscribePrivateChannel(QString channel) {

		channel = "private-" + channel;

		authPusherChannel(channel, [=](QString auth) {
			QString message = QString("{\"event\":\"pusher:subscribe\",\"data\":{\"auth\":\""+auth+"\",\"channel\":\"%1\"}}").arg(channel);
			sendMessage(message);
		});
	}


	void Broadcast::onChannelMessage(QString message) {

		// get json
		QJsonParseError error;
		QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &error);

		if(error.error != QJsonParseError::NoError) {
			qDebug() << "Error parsing channel message: " << error.errorString();
			return;
		}

		// get data
		QJsonObject obj = doc.object();
		QString channel = obj.value("channel").toString().replace("private-","");
		QString event = obj.value("event").toString().replace("client-","");
		QJsonObject data = obj.value("data").toObject();

		emit channelMessageReceived(channel, event, data);
	}


	void Broadcast::sendPrivateChannel(QString channel, QString event, QJsonObject data) {

		// create data
		QJsonObject messageData = QJsonObject();
		messageData["channel"] = "private-"+channel;
		messageData["event"] = "client-"+event;
		messageData["data"] = data;

		// send message
		sendMessage(messageData);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	SETTINGS
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void Broadcast::initSettings() {

		// connect settings
		connect(Settings::instance(), &Settings::settingsUpdated, this, &Broadcast::onSettingsUpdated);
	}


	void Broadcast::onSettingsUpdated(QString key, QVariant value) {

		// update base url
		if(key == "websocket_port") { 
			webSocketPort = value.toString(); 
		}
	}