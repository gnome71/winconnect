#include <QDebug>
#include <QtNetwork>

#include "udplistener.h"
#include "core/kdeconnectconfig.h"
#include "core/daemon.h"
#include "core/backends/loopback/loopbackdevicelink.h"

#define MIN_VERSION_WITH_SSL_SUPPORT 6

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

	connect(udpSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));
	connect(udpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
			this, SLOT(processStatusChanged(QAbstractSocket::SocketState)));
}

UdpListenerThread::~UdpListenerThread()
{
	qDebug() << "~UdpListenerThread()";
}

QUdpSocket* UdpListenerThread::getSocket()
{
	return this->udpSocket;
}

void UdpListenerThread::processPendingDatagrams()
{
	while (udpSocket->hasPendingDatagrams()) {

		QByteArray datagram;
		datagram.resize(udpSocket->pendingDatagramSize());
		QHostAddress sender;

		udpSocket->readDatagram(datagram.data(), datagram.size(), &sender);

		NetworkPackage* receivedPackage = new NetworkPackage(QString::null);
		bool success = NetworkPackage::unserialize(datagram, receivedPackage);

		//qDebug() << "udp connection from " << sender.toString();
		//qDebug() << "Datagram " << datagram.data() ;

		if (!success || receivedPackage->type() != PACKAGE_TYPE_IDENTITY) {
			delete receivedPackage;
			continue;
		}

		if (receivedPackage->get<QString>("deviceId") == KdeConnectConfig::deviceId()) {
			qDebug() << "Ignoring my own broadcast";
			emit logMe(QtMsgType::QtInfoMsg, "UdpListen ", "Ignoring my own broadcast.");
			delete receivedPackage;
			continue;
		}

		int tcpPort = receivedPackage->get<int>("tcpPort", PORT);

		QString msg = "Received Udp identity package from " + sender.toString()
				+ " asking for a tcp connection on port " + QString::number(tcpPort);
		qDebug() << msg;
		emit logMe(QtMsgType::QtInfoMsg, "UdpListen ", msg);

		QSslSocket* socket = new QSslSocket(this);
		receivedIdentityPackages[socket].np = receivedPackage;
		receivedIdentityPackages[socket].sender = sender;
		connect(socket, SIGNAL(connected()), this, SLOT(connected()));
		connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectError()));
		connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(processStatusChanged(QAbstractSocket::SocketState)));
		socket->connectToHost(sender, tcpPort);
	}
}

// We received a UDP package and answered by connecting to them by TCP.
// This gets called on a succesful connection.
void UdpListenerThread::connected()
{
	KdeConnectConfig* config = new KdeConnectConfig();
	qDebug() << "Socket connected";
	emit logMe(QtMsgType::QtInfoMsg, "UdpListen ", "Socket connected");

	QSslSocket* socket = qobject_cast<QSslSocket*>(sender());
	if (!socket) return;
	disconnect(socket, SIGNAL(connected()), this, SLOT(connected()));
	disconnect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectError()));

	configureSocket(socket);

	// If socket disconnects due to any reason after connection, link on ssl failure
	connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));

	NetworkPackage* receivedPackage = receivedIdentityPackages[socket].np;
	const QString& deviceId = receivedPackage->get<QString>("deviceId");
	qDebug() << "Connected";
	//qDebug() << "Socket is writeable:" << socket->isWritable() ? "true" : "false";

	// If network is on ssl, do not believe when they are connected,
	// believe when handshake is completed
	NetworkPackage np2(QString::null);
	NetworkPackage::createIdentityPackage(&np2);
	socket->write(np2.serialize());
	bool success = socket->waitForBytesWritten();

	if (success) {

		qDebug() << "TCP connection done (i'm the existing device)";
		emit logMe(QtMsgType::QtInfoMsg, "UdpListen ", "TCP connection done (I'm the existing device)");

		// if ssl supported
		if (receivedPackage->get<int>("protocolVersion") >= MIN_VERSION_WITH_SSL_SUPPORT) {

			bool isDeviceTrusted = config->trustedDevices().contains(deviceId);
			configureSslSocket(socket, deviceId, isDeviceTrusted);

			qDebug() << "Starting server ssl (I'm the client TCP socket)";
			emit logMe(QtMsgType::QtInfoMsg, "UdpListen ", "Starting server ssl (I'm the client TCP socket)");

			connect(socket, SIGNAL(encrypted()), this, SLOT(encrypted()));

			if (isDeviceTrusted) {
				connect(socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));
			}

			socket->startServerEncryption();

			return; // Return statement prevents from deleting received package, needed in slot "encrypted"
		} else {
			qWarning() << receivedPackage->get<QString>("deviceName") << "uses an old protocol version, this won't work";
			emit logMe(QtMsgType::QtWarningMsg, "UdpListen ", "Device uses an old protocol");
			//addLink(deviceId, socket, receivedPackage, LanDeviceLink::Remotely);
		}

	} else {
		//I think this will never happen, but if it happens the deviceLink
		//(or the socket that is now inside it) might not be valid. Delete them.
		qDebug() << "Fallback (2), try reverse connection (send udp packet)";
		emit logMe(QtMsgType::QtInfoMsg, "UdpListen ", "Fallback (2). trying reverse connection");
		udpSocket->writeDatagram(np2.serialize(), receivedIdentityPackages[socket].sender, PORT);
	}

	delete receivedIdentityPackages.take(socket).np;
	//We don't delete the socket because now it's owned by the LanDeviceLink
}

void UdpListenerThread::encrypted()
{
	qDebug() << "Socket succesfully established an SSL connection";
	emit logMe(QtMsgType::QtInfoMsg, "UdpListen ", "Socket succesfully established an SSL connection");

	QSslSocket* socket = qobject_cast<QSslSocket*>(sender());
	if (!socket) return;
	disconnect(socket, SIGNAL(encrypted()), this, SLOT(encrypted()));
	disconnect(socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));

	Q_ASSERT(socket->mode() != QSslSocket::UnencryptedMode);

	//TODO: LanDeviceLink::ConnectionStarted connectionOrigin = (socket->mode() == QSslSocket::SslClientMode)? LanDeviceLink::Locally : LanDeviceLink::Remotely;

	NetworkPackage* receivedPackage = receivedIdentityPackages[socket].np;
	const QString& deviceId = receivedPackage->get<QString>("deviceId");

	//addLink(deviceId, socket, receivedPackage, connectionOrigin);

	// Copied from connected slot, now delete received package
	delete receivedIdentityPackages.take(socket).np;
}

void UdpListenerThread::sslErrors(const QList<QSslError>& errors)
{
	QSslSocket* socket = qobject_cast<QSslSocket*>(sender());
	if (!socket) return;

	disconnect(socket, SIGNAL(encrypted()), this, SLOT(encrypted()));
	disconnect(socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));

	Q_FOREACH (const QSslError &error, errors) {
		switch (error.error()) {
			case QSslError::CertificateSignatureFailed:
			case QSslError::CertificateNotYetValid:
			case QSslError::CertificateExpired:
			case QSslError::CertificateUntrusted:
			case QSslError::SelfSignedCertificate: {
				qDebug() << "Failing due to " << error.errorString();
				emit logMe(QtMsgType::QtCriticalMsg, "UdpListen ", "SSL error: " + error.errorString());
				// Due to simultaneous multiple connections, it may be possible that device instance does not exist anymore
				Device *device = Daemon::instance()->getDevice(socket->peerVerifyName());
				if (device != Q_NULLPTR) {
					device->unpair();
				}
				break;
			}
			default:
				continue;
		}
	}

	delete receivedIdentityPackages.take(socket).np;
	// Socket disconnects itself on ssl error and will be deleted by deleteLater slot, no need to delete manually
}

void UdpListenerThread::configureSslSocket(QSslSocket* socket, const QString& deviceId, bool isDeviceTrusted)
{
	KdeConnectConfig* config = new KdeConnectConfig();

	// Setting supported ciphers manually
	// Top 3 ciphers are for new Android devices, botton two are for old Android devices
	// FIXME : These cipher suites should be checked whether they are supported or not on device
	QList<QSslCipher> socketCiphers;
	socketCiphers.append(QSslCipher("ECDHE-ECDSA-AES256-GCM-SHA384"));
	socketCiphers.append(QSslCipher("ECDHE-ECDSA-AES128-GCM-SHA256"));
	socketCiphers.append(QSslCipher("ECDHE-RSA-AES128-SHA"));
	socketCiphers.append(QSslCipher("RC4-SHA"));
	socketCiphers.append(QSslCipher("RC4-MD5"));

	// Configure for ssl
	QSslConfiguration sslConfig;
	sslConfig.setCiphers(socketCiphers);
	sslConfig.setProtocol(QSsl::TlsV1_0);

	socket->setSslConfiguration(sslConfig);
	socket->setLocalCertificate(config->certificate());
	socket->setPrivateKey(config->privateKeyPath());
	socket->setPeerVerifyName(deviceId);

	if (isDeviceTrusted) {
		QString certString = config->getDeviceProperty(deviceId, "certificate", QString());
		socket->addCaCertificate(QSslCertificate(certString.toLatin1()));
		socket->setPeerVerifyMode(QSslSocket::VerifyPeer);
	} else {
		socket->setPeerVerifyMode(QSslSocket::QueryPeer);
	}

	//Usually SSL errors are only bad for trusted devices. Uncomment this section to log errors in any case, for debugging.
	QObject::connect(socket, static_cast<void (QSslSocket::*)(const QList<QSslError>&)>(&QSslSocket::sslErrors), [](const QList<QSslError>& errors)
	{
		Q_FOREACH (const QSslError &error, errors) {
			QString msg = "SSL debug: " + error.errorString();
			qDebug() << msg;
			//emit logMe(QtMsgType::QtInfoMsg, "UdpListen:", msg);
		}
	});
}

void UdpListenerThread::configureSocket(QSslSocket* socket) {

	socket->setSocketOption(QAbstractSocket::KeepAliveOption, QVariant(1));

	#ifdef TCP_KEEPIDLE
		// time to start sending keepalive packets (seconds)
		int maxIdle = 10;
		setsockopt(socket->socketDescriptor(), IPPROTO_TCP, TCP_KEEPIDLE, &maxIdle, sizeof(maxIdle));
	#endif

	#ifdef TCP_KEEPINTVL
		// interval between keepalive packets after the initial period (seconds)
		int interval = 5;
		setsockopt(socket->socketDescriptor(), IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
	#endif

	#ifdef TCP_KEEPCNT
		// number of missed keepalive packets before disconnecting
		int count = 3;
		setsockopt(socket->socketDescriptor(), IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count));
	#endif

}


void UdpListenerThread::connectError()
{
	QSslSocket* socket = qobject_cast<QSslSocket*>(sender());
	if (!socket) return;
	disconnect(socket, SIGNAL(connected()), this, SLOT(connected()));
	disconnect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectError()));

	qDebug() << "Fallback (1), try reverse connection (send udp packet)" << socket->errorString();
	emit logMe(QtMsgType::QtInfoMsg, "UdpListen ", "Fallback (1), try reverse connection (send udp packet) " + socket->errorString());
	NetworkPackage np(QString::null);
	NetworkPackage::createIdentityPackage(&np);
	np.set("tcpPort", mTcpPort);
	udpSocket->writeDatagram(np.serialize(), receivedIdentityPackages[socket].sender, PORT);

	//The socket we created didn't work, and we didn't manage
	//to create a LanDeviceLink from it, deleting everything.
	delete receivedIdentityPackages.take(socket).np;
	delete socket;
}


void UdpListenerThread::processStatusChanged(QAbstractSocket::SocketState socketState)
{
	switch(socketState) {
		case QAbstractSocket::SocketState::ConnectingState:
			emit status("connecting ...");
			emit logMe(QtMsgType::QtInfoMsg, "UdpListen ", "Connecting");
			break;
		case QAbstractSocket::SocketState::ConnectedState:
			emit status("connected.");
			emit logMe(QtMsgType::QtInfoMsg, "UdpListen ", "Connected");
			break;
		case QAbstractSocket::SocketState::HostLookupState:
			emit status("looking up host ...");
			emit logMe(QtMsgType::QtInfoMsg, "UdpListen ", "Looking up host");
			break;
		case QAbstractSocket::SocketState::ListeningState:
			emit status("listening ...");
			emit logMe(QtMsgType::QtInfoMsg, "UdpListen ", "Listening");
			break;
		case QAbstractSocket::SocketState::UnconnectedState:
			emit status("not connected.");
			emit logMe(QtMsgType::QtInfoMsg, "UdpListen ", "Not connected");
			break;
		case QAbstractSocket::SocketState::ClosingState:
			emit status("closing connection ...");
			emit logMe(QtMsgType::QtInfoMsg, "UdpListen ", "Closing connection");
			break;
		case QAbstractSocket::SocketState::BoundState:
			emit status("socket bound.");
			emit logMe(QtMsgType::QtInfoMsg, "UdpListen ", "Socket bound");
			break;
		default:
			emit status("unknown state.");
			break;
	}
}
