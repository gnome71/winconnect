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


struct KdeConnectConfigPrivate {
	QCA::Initializer mQcaInitializer;

	QCA::PrivateKey privateKey;
	QCA::PublicKey publicKey;
	// Use QSslCertificate instead of QCA::Certificate due to compatibility with QSslSocket
	QSslCertificate certificate;

	//QSettings* config;
	//QSettings* trusted_devices;
};

KdeConnectConfig::KdeConnectConfig()
{
	//QCA::Initializer mQcaInitializer;

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
	QSettings trusted_devices;

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
	QCA::PrivateKey privateKey;
	if(privKey.exists() && privKey.open(QIODevice::ReadOnly)) {
		privateKey = QCA::PrivateKey::fromPEM(privKey.readAll());
		//qCDebug(kcQca) << "Opened private key: " << keyPath;
		if(privateKey.isNull())
			qCDebug(kcQca) << "load: privateKey.isNull";
		privKey.close();
	}
	else {
		privateKey = QCA::KeyGenerator().createRSA(2048);
		if(!privKey.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
			qCDebug(kcQca) << "Could not store private key file: " << privKey.fileName();
		}
		else {
			int len = privKey.write(privateKey.toPEM().toLatin1());
			qCDebug(kcQca) << "Created private key with length: " << len;
			privKey.close();
		}
	}

	// Load or register certificate if not there
	QString certPath = certificatePath();
	QFile cert(certPath);
	QSslCertificate certificate;
	if(cert.exists() && cert.open(QIODevice::ReadOnly)) {
		if(!QSslCertificate::fromPath(certPath, QSsl::Pem).at(0).isNull()) {
			certificate = QSslCertificate::fromPath(certPath).at(0);
			//qCDebug(kcQca) << "Opened Cert: " << certPath;
		}
		else { qCDebug(kcQca) << "Unable to open: " + certPath; }

		cert.close();
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
		else {
			cert.write(certificate.toPem());
			cert.close();
		}
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

QCA::PrivateKey KdeConnectConfig::privateKey()
{
	QFile privKey(privateKeyPath());
	QCA::PrivateKey pk;
	if(privKey.exists() && privKey.open(QIODevice::ReadOnly)) {
		pk = QCA::PrivateKey::fromPEM(privKey.readAll());
		//qCDebug(kcQca) << "Opened private key: " << privateKeyPath();
		if(pk.isNull())
			qCDebug(kcQca) << "load: privateKey.isNull";
	}
	privKey.close();
	return pk;
}

QCA::PublicKey KdeConnectConfig::publicKey()
{
	QFile privKey(privateKeyPath());
	QCA::PrivateKey privateKey;
	if(privKey.exists() && privKey.open(QIODevice::ReadOnly)) {
		privateKey = QCA::PrivateKey::fromPEM(privKey.readAll());
		//qCDebug(kcQca) << "Opened private key: " << privateKeyPath();
		if(privateKey.isNull())
			qCDebug(kcQca) << "load: privateKey.isNull";
	}
	privKey.close();
	return privateKey.toPublicKey();
}

QString KdeConnectConfig::certificatePath()
{
	return this->baseConfigDir().absoluteFilePath("certificate.pem");
}

QSslCertificate KdeConnectConfig::certificate()
{
	QFile cert(certificatePath());
	QSslCertificate certificate;
	if(cert.exists() && cert.open(QIODevice::ReadOnly)) {
		if(!QSslCertificate::fromPath(certificatePath(), QSsl::Pem).at(0).isNull()) {
			certificate = QSslCertificate::fromPath(certificatePath()).at(0);
			//qCDebug(kcQca) << "Opened Cert: " << certificatePath();
		}
		else { qCDebug(kcQca) << "Unable to open: " + certificatePath(); }

		cert.close();
	}
	return certificate;
}

void KdeConnectConfig::setName(QString name)
{
	QSettings config;
	config.sync();
	config.setValue("my/name", name);
	config.sync();
}

QStringList KdeConnectConfig::trustedDevices()
{
	QSettings trusted_devices;
	trusted_devices.sync();
	const QStringList& list = trusted_devices.childGroups();
	return list;
}

void KdeConnectConfig::addTrustedDevice(const QString &id, const QString &name, const QString &type)
{
	QSettings trusted_devices;
	trusted_devices.sync();
	trusted_devices.beginGroup("trustedDevices");
	trusted_devices.setValue("id/name", name);
	trusted_devices.setValue("id/type", type);
	trusted_devices.endGroup();

	QDir().mkpath(deviceConfigDir(id).path());
}

KdeConnectConfig::DeviceInfo KdeConnectConfig::getTrustedDevice(const QString &id)
{
	QSettings trusted_devices;
	trusted_devices.sync();
	trusted_devices.beginGroup("trustedDevices");
	KdeConnectConfig::DeviceInfo info;
	info.deviceName = trusted_devices.value("id/name", QLatin1String("unnamed")).toString();
	info.deviceType = trusted_devices.value("id/type", QLatin1String("unknown")).toString();
	trusted_devices.endGroup();

	return info;
}

void KdeConnectConfig::removeTrustedDevice(const QString &deviceId)
{
	QSettings trusted_devices;
	trusted_devices.sync();
	trusted_devices.remove(deviceId);
	trusted_devices.sync();
	// We do not remove the config files
}

void KdeConnectConfig::setDeviceProperty(QString deviceId, QString key, QString value)
{
	QSettings trusted_devices;
	trusted_devices.sync();
	trusted_devices.beginGroup("trustedDevices");
	trusted_devices.setValue("deviceId/" + key, value);
	trusted_devices.endGroup();
	trusted_devices.sync();
}

QString KdeConnectConfig::getDeviceProperty(QString deviceId, QString key, QString defaultValue)
{
	QSettings trusted_devices;
	trusted_devices.sync();
	QString value;
	trusted_devices.beginGroup("trustedDevices/" + deviceId);
	value = trusted_devices.value(key, defaultValue).toString();
	trusted_devices.endGroup();
	return value;
}

QString KdeConnectConfig::getQcaInfo() {
	QCA::PrivateKey pk = privateKey();
	QSslCertificate cert = certificate();
	QString msg = "QCA Diagnostic:\n" + QCA::pluginDiagnosticText();
	msg += "QCA capabilities:\n" + QCA::supportedFeatures().join(", ");
	msg += "\n";
	msg += pk.toPEM().toLatin1();
	msg += pk.toPublicKey().toPEM().toLatin1();
	msg += cert.toPem();
	msg += "\n";
	if(pk.canDecrypt())
		msg += "privateKey canDecrypt ";
	if(pk.canEncrypt())
		msg += "canEncrypt ";
	if(pk.canExport())
		msg += "canExport ";
	if(pk.canSign())
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
	//QSettings config;
	//config.sync();

	// device configuration directory
	QString deviceConfigPath = baseConfigDir().absoluteFilePath(deviceId);
	return QDir(deviceConfigPath);
}

QDir KdeConnectConfig::pluginConfigDir(const QString &deviceId, const QString &pluginName)
{
	QString deviceConfigPath = baseConfigDir().absoluteFilePath(deviceId);
	QString pluginConfigDir = QDir(deviceConfigPath).absoluteFilePath(pluginName);
	return QDir(pluginConfigDir);
}

