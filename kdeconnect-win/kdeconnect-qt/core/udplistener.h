#ifndef UDPLISTENER_H
#define UDPLISTENER_H

#include <QObject>
#include <QUdpSocket>
#include <QSslSocket>

#include "core/networkpackage.h"
#include "core/backends/lan/server.h"

class UdpListenerThread : public QObject
{
	Q_OBJECT

public:
	explicit UdpListenerThread(QObject *parent = Q_NULLPTR);
	~UdpListenerThread();

	QUdpSocket* getSocket();

	static void configureSslSocket(QSslSocket* socket, const QString& deviceId, bool isDeviceTrusted);
	static void configureSocket(QSslSocket* socket);

	const static quint16 PORT = 1716;

signals:
	void newIdentification(const QString &idPackage);
	void error(int socketError, const QString &message);
	void status(QString);
	void logMe(QtMsgType type, const QString &prefix, const QString &msg);

public slots:
	void connected();
	void connectError();
	void encrypted();

private slots:
	void processPendingDatagrams();
	void processStatusChanged(QAbstractSocket::SocketState socketState);
	void sslErrors(const QList<QSslError>& errors);

private:
	Server* mServer;
	QHostAddress bindAddress;
	QUdpSocket* udpSocket;
	quint16 mTcpPort;

	struct PendingConnect {
		NetworkPackage* np;
		QHostAddress sender;
	};
	QMap<QSslSocket*, PendingConnect> receivedIdentityPackages;

};

#endif // UDPLISTENER_H
