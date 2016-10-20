#ifndef UDPLISTENER_H
#define UDPLISTENER_H

#include <QObject>
#include <QUdpSocket>

class UdpListenerThread : public QObject
{
	Q_OBJECT

public:
	explicit UdpListenerThread(QObject *parent = Q_NULLPTR);
	~UdpListenerThread();

signals:
	void newIdentification(const QString &idPackage);
	void error(int socketError, const QString &message);
	void status(QString);

private slots:
	void processPendingDatagrams();
	void processStatusChanged(QAbstractSocket::SocketState socketState);

private:
	QHostAddress bindAddress;
	QUdpSocket *udpSocket;
};

#endif // UDPLISTENER_H
