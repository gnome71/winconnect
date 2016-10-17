#include "kdeconnectconfig.h"
#include "kclogger.h"
#include "deviceidhelper.h"

#include <QFile>
#include <QDebug>
#include <QLoggingCategory>
#include <QFileInfo>
#include <QUuid>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QHostInfo>
#include <QSettings>
#include <QSslCertificate>
#include <QtCrypto>
#include <QSslCertificate>

// not used
//struct KdeConnectConfigPrivate {
//	QCA::Initializer mQcaInitializer;

//	QCA::PrivateKey privateKey;
//	// Use QSslCertificate instead of QCA::Certificate due to compatibility with QSslSocket
//	QSslCertificate certificate;

//	QSettings* config;
//	QSettings* trusted_devices;
//};

KdeConnectConfig::KdeConnectConfig()
{

	QCA::Initializer mQcaInitializer;

	if(!QCA::isSupported("rsa")) {
		qCDebug(kcQca) << "RSA not supported";
		return;
	}
	else if(!QCA::isSupported("pkey") ||
			!QCA::PKey::supportedIOTypes().contains(QCA::PKey::RSA)) {
		qCDebug(kcQca) << "PKEY not supported";
		return;
	}

	QSettings::setDefaultFormat(QSettings::IniFormat);
	QSettings config;
	config.sync();

	//Register my own id if not there yet
	if(!config.contains("my/id")) {
		QString uuid = "";
		uuid = QUuid::createUuid().toString();
		DeviceIdHelper::filterNonExportableCharacters(uuid);
		config.setValue("my/id", uuid);
		config.sync();
		qCDebug(kcQca) << "My id:" << uuid;
	}

	// Register my own name if not there yet
	if(!config.contains("my/name")) {
		QString n = qgetenv("USERNAME");
		QString h = qgetenv("COMPUTERNAME");
		QString name = n + "@" + h;
		config.setValue("my/name", name);
		config.sync();
	}

	// Register my own deviceType as Desktop hardcoded
	if (!config.contains("my/deviceType")) {
		QString deviceType = "desktop";
		qCDebug(kcQca) << "My deviceType: " << deviceType;
		config.setValue("my/deviceType", deviceType);
		config.sync();
	}

	// Load or register new private key if not there
	QString keyPath = privateKeyPath();
	QFile privKey(keyPath);
	if(privKey.exists() && privKey.open(QIODevice::ReadOnly)) {
		privateKey = QCA::PrivateKey::fromPEM(privKey.readAll());
		qCDebug(kcQca) << "Opened private key: " << keyPath;
		if(privateKey.isNull())
			qCDebug(kcQca) << "load: privateKey.isNull";
	}
	else {
		privateKey = QCA::KeyGenerator().createRSA(2048);
		if(!privKey.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
			qCDebug(kcQca) << "Could not store private key file: " << privKey.fileName();
		}
		else {
			int len = privKey.write(privateKey.toPEM().toLatin1());
			qCDebug(kcQca) << "write private key length: " << len;
		}
	}

	publicKey = privateKey.toPublicKey();
	if(publicKey.isNull())
		qCDebug(kcQca) << "create; publicKey.isNull";

	// Load or register certificate if not there
	QString certPath = certificatePath();
	QFile cert(certPath);
	if(cert.exists() && cert.open(QIODevice::ReadOnly)) {
		if(!QSslCertificate::fromPath(certPath, QSsl::Pem).at(0).isNull()) {
			certificate = QSslCertificate::fromPath(certPath).at(0);
			qCDebug(kcQca) << "Opened Cert: " << certPath;
		}
		else { qCDebug(kcQca) << "Unable to open: " + certPath; }
	}
	else {
		// create certificate
		QCA::CertificateOptions certificateOptions = QCA::CertificateOptions();
		QDateTime startTime = QDateTime::currentDateTime().addYears(-1);
		QDateTime endTime = startTime.addYears(10);
		QCA::CertificateInfo certificateInfo;
		certificateInfo.insert(QCA::CommonName,deviceId());
		certificateInfo.insert(QCA::Organization, "KCW");
		certificateInfo.insert(QCA::OrganizationalUnit,"Kde connect win");
		certificateOptions.setInfo(certificateInfo);
		certificateOptions.setFormat(QCA::PKCS10);
		certificateOptions.setSerialNumber(QCA::BigInteger(10));
		certificateOptions.setValidityPeriod(startTime, endTime);

		certificate = QSslCertificate(QCA::Certificate(certificateOptions, privateKey).toPEM().toLatin1());

		if(!cert.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
			qCDebug(kcQca) << "Could not store certificate file: " + cert.fileName();
		}
		else { cert.write(certificate.toPem()); }
	}
}

QString KdeConnectConfig::deviceId()
{
	QSettings config;
	config.sync();
	QString ret = config.value("my/id").toString();
	return ret;
}

QString KdeConnectConfig::name()
{
	QSettings config;
	config.sync();
	return config.value("my/name").toString();
}

QString KdeConnectConfig::deviceType()
{
	QSettings config;
	config.sync();
	return config.value("my/deviceType").toString();
}

QString KdeConnectConfig::privateKeyPath()
{

	return this->baseConfigDir().absoluteFilePath("privateKey.pem");
}

QCA::PrivateKey KdeConnectConfig::getPrivateKey()
{
	return privateKey;
}

QCA::PublicKey KdeConnectConfig::getPublicKey()
{
	return privateKey.toPublicKey();
	//return publicKey.toPrivateKey();
}

QString KdeConnectConfig::certificatePath()
{
	return this->baseConfigDir().absoluteFilePath("certificate.pem");
}

QSslCertificate KdeConnectConfig::getCertificate()
{
	return certificate;
}

void KdeConnectConfig::setName(QString name)
{
	QSettings config;
	config.sync();
	config.setValue("my/name", name);
	config.sync();
	emit nameChanged(name);
}

QStringList KdeConnectConfig::trustedDevices()
{
	QStringList todo;
	todo << "to" << "do";
	return todo;
}

void KdeConnectConfig::removeTrustedDevice(const QString &id)
{

}

void KdeConnectConfig::addTrustedDevice(const QString &id, const QString &name, const QString &type)
{

}

KdeConnectConfig::DeviceInfo KdeConnectConfig::getTrustedDevice(const QString &id)
{
	KdeConnectConfig::DeviceInfo trustdevice;
	return trustdevice;
}

void KdeConnectConfig::setDeviceProperty(QString deviceId, QString name, QString value)
{

}

QString KdeConnectConfig::getDeviceProperty(QString deviceId, QString name, QString defaultValue)
{
	return "todo";
}

QString KdeConnectConfig::getQcaInfo() {
	QString msg = "QCA Diagnostic:\n" + QCA::pluginDiagnosticText();
	msg += "QCA capabilities:\n" + QCA::supportedFeatures().join(", ");
	msg += "\n";
	msg += privateKey.toPEM().toLatin1();
	msg += privateKey.toPublicKey().toPEM().toLatin1();
	msg += certificate.toPem();
	msg += "\n";
	if(privateKey.canDecrypt())
		msg += "privateKey canDecrypt ";
	if(privateKey.canEncrypt())
		msg += "canEncrypt ";
	if(privateKey.canExport())
		msg += "canExport ";
	if(privateKey.canSign())
		msg += "canSign ";
	msg += "\n";
	return msg;
}

/**
 * @brief KdeConnectConfig::baseConfigDir
 * @return QDir
 */
QDir KdeConnectConfig::baseConfigDir()
{
	QSettings config;
	config.sync();

	// base configuration directory without filename
	QFileInfo info(config.fileName());
	QDir bcd(info.absolutePath());

	return bcd;
}

/**
 * @brief KdeConnectConfig::deviceConfigDir
 * returns also the base path (using one config file)
 * @param deviceId
 * @return QDir
 */
QDir KdeConnectConfig::deviceConfigDir(const QString &deviceId)
{
	QSettings config;
	config.sync();

	// device configuration directory
	QDir dcd(baseConfigDir().path());

	return dcd;
}

QDir KdeConnectConfig::pluginConfigDir(const QString &deviceId, const QString &pluginName)
{
	return QDir("todo");
}

