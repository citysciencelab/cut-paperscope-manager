/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#pragma once

	// Qt
	#include <QObject>
	#include <QWebSocket>
	#include <QAbstractSocket>
	#include <QMap>
	#include <QJsonObject>

	// App
	#include "Settings.h"



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CLASS DECLARATION
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


class Broadcast : public QObject {
	
	Q_OBJECT

	public:

		static Broadcast* instance();
		~Broadcast();

		// websocket
		void sendMessage(QString message);
		void sendMessage(QJsonObject data);

		// pusher
		void sendPrivateChannel(QString channel, QString event, QJsonObject data);


	private:

		// singleton
		static bool instanceFlag;
		static Broadcast* broadcastInstance;
		Broadcast();

		// websocket
		void connectWebSocket();
		QWebSocket *webSocket;
		QString webSocketPort;
		QString webSocketAppId;
		QString webSocketId;
		QMap<QString,QString> webSocketAuth;

		// pusher
		void authPusherChannel(QString channel, std::function<void(QString auth)> callback);

		// settings
		void initSettings();


	signals:

		// websocket
		void connected();
		void closed();
		void messageReceived(QString message);
		void channelMessageReceived(QString channel, QString event, QJsonObject data);


	public slots:

		// websocket
		void onWebSocketConnected();
		void onWebSocketClosed();
		void onWebSocketError(QAbstractSocket::SocketError error);
   		void onMessage(QString message);

		// pusher
		void sendPing();
		void subscribePrivateChannel(QString channel);
		void onChannelMessage(QString message);

		// settings
		void onSettingsUpdated(QString key, QVariant value);
};

