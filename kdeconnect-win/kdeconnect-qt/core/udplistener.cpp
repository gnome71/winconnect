#include <QDebug>
#include <QtNetwork>

#include "udplistener.h"


UdpListenerThread::UdpListenerThread(QObject *parent)
	: QObject(parent)
{
	bindAddress = QHostAddress::AnyIPv4;
	quint16 bindPort = 1716;

		udpSocket = new QUdpSocket(this);
		if(!udpSocket->bind(bindAddress, bindPort, QUdpSocket::ShareAddress)) {
			udpSocket->close();
			qDebug() << "Bind error" << udpSocket->errorString();
			return;
		}
		qDebug() << "bindIP:" << udpSocket->localAddress();
		qDebug() << "bindPort:" << udpSocket->localPort();
		qDebug() << "Socket exists.";
		qDebug() << "bindIP:" << udpSocket->localAddress();
		qDebug() << "bindPort:" << udpSocket->localPort();

	connect(udpSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));
	connect(udpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
			this, SLOT(processStatusChanged(QAbstractSocket::SocketState)));
}

UdpListenerThread::~UdpListenerThread()
{
	qDebug() << "~UdpListenerThread()";
}

void UdpListenerThread::processPendingDatagrams()
{
	QByteArray datagram;
	QHostAddress peerAddr;
	quint16 peerPort;
	datagram.resize(udpSocket->pendingDatagramSize());
	udpSocket->readDatagram(datagram.data(), datagram.size(), &peerAddr, &peerPort);
	emit newIdentification(datagram.data());
	qDebug() << "New datagram: " << datagram.data();
	qDebug() << "peerAddr:" << udpSocket->peerAddress();
	qDebug() << "peerPort:" << udpSocket->peerPort();
}

void UdpListenerThread::processStatusChanged(QAbstractSocket::SocketState socketState)
{
	switch(socketState) {
		case QAbstractSocket::SocketState::ConnectingState:
			emit status("connecting ...");
			break;
		case QAbstractSocket::SocketState::ConnectedState:
			emit status("connected.");
			break;
		case QAbstractSocket::SocketState::HostLookupState:
			emit status("looking up host ...");
			break;
		case QAbstractSocket::SocketState::ListeningState:
			emit status("listening ...");
			break;
		case QAbstractSocket::SocketState::UnconnectedState:
			emit status("not connected.");
			break;
		case QAbstractSocket::SocketState::ClosingState:
			emit status("closing connection ...");
			break;
		case QAbstractSocket::SocketState::BoundState:
			emit status("socket bound.");
			break;
		default:
			emit status("unknown state.");
			break;
	}
}
