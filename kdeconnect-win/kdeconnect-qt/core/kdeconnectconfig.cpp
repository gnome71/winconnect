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

	QSettings* config;
	QSettings* trusted_devices;
};

KdeConnectConfig::KdeConnectConfig()
	: d(new KdeConnectConfigPrivate)
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
	//QSettings config;
	//config.sync();

	//Register my own id if not there yet
	if(!d->config->contains("my/id")) {
		QString uuid = "";
		uuid = QUuid::createUuid().toString();
		DeviceIdHelper::filterNonExportableCharacters(uuid);
		d->config->setValue("my/id", uuid);
		d->config->sync();
		qCDebug(kcQca) << "My id:" << uuid;
	}

	// Register my own name if not there yet
	if(!d->config->contains("my/name")) {
		QString n = qgetenv("USERNAME");
		QString h = qgetenv("COMPUTERNAME");
		QString name = n + "@" + h;
		d->config->setValue("my/name", name);
		d->config->sync();
	}

	// Register my own deviceType as Desktop hardcoded
	if (!d->config->contains("my/deviceType")) {
		QString deviceType = "desktop";
		qCDebug(kcQca) << "My deviceType: " << deviceType;
		d->config->setValue("my/deviceType", deviceType);
		d->config->sync();
	}

	// Load or register new private key if not there
	QString keyPath = privateKeyPath();
	QFile privKey(keyPath);
	if(privKey.exists() && privKey.open(QIODevice::ReadOnly)) {
		d->privateKey = QCA::PrivateKey::fromPEM(privKey.readAll());
		qCDebug(kcQca) << "Opened private key: " << keyPath;
		if(d->privateKey.isNull())
			qCDebug(kcQca) << "load: privateKey.isNull";
	}
	else {
		d->privateKey = QCA::KeyGenerator().createRSA(2048);
		if(!privKey.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
			qCDebug(kcQca) << "Could not store private key file: " << privKey.fileName();
		}
		else {
			int len = privKey.write(d->privateKey.toPEM().toLatin1());
			qCDebug(kcQca) << "write private key length: " << len;
		}
	}

	d->publicKey = d->privateKey.toPublicKey();
	if(d->publicKey.isNull())
		qCDebug(kcQca) << "create; publicKey.isNull";

	// Load or register certificate if not there
	QString certPath = certificatePath();
	QFile cert(certPath);
	if(cert.exists() && cert.open(QIODevice::ReadOnly)) {
		if(!QSslCertificate::fromPath(certPath, QSsl::Pem).at(0).isNull()) {
			d->certificate = QSslCertificate::fromPath(certPath).at(0);
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

		d->certificate = QSslCertificate(QCA::Certificate(certificateOptions, d->privateKey).toPEM().toLatin1());

		if(!cert.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
			qCDebug(kcQca) << "Could not store certificate file: " + cert.fileName();
		}
		else { cert.write(d->certificate.toPem()); }
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
	//QSettings config;
	//config.sync();
	return d->config->value("my/name").toString();
}

QString KdeConnectConfig::deviceType()
{
	//QSettings config;
	//config.sync();
	return d->config->value("my/deviceType").toString();
}

QString KdeConnectConfig::privateKeyPath()
{

	return this->baseConfigDir().absoluteFilePath("privateKey.pem");
}

QCA::PrivateKey KdeConnectConfig::privateKey()
{
	return d->privateKey;
}

QCA::PublicKey KdeConnectConfig::publicKey()
{
	return d->privateKey.toPublicKey();
}

QString KdeConnectConfig::certificatePath()
{
	return this->baseConfigDir().absoluteFilePath("certificate.pem");
}

QSslCertificate KdeConnectConfig::certificate()
{
	return d->certificate;
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
	const QStringList& list = d->trusted_devices->childGroups();
	return list;
}

void KdeConnectConfig::addTrustedDevice(const QString &id, const QString &name, const QString &type)
{
	d->trusted_devices->beginGroup(id);
	d->trusted_devices->setValue("name", name);
	d->trusted_devices->setValue("type", type);
	d->trusted_devices->endGroup();
	d->trusted_devices->sync();

	QDir().mkpath(deviceConfigDir(id).path());
}

KdeConnectConfig::DeviceInfo KdeConnectConfig::getTrustedDevice(const QString &id)
{
	d->trusted_devices->beginGroup(id);
	KdeConnectConfig::DeviceInfo info;
	info.deviceName = d->trusted_devices->value("name", QLatin1String("unnamed")).toString();
	info.deviceType = d->trusted_devices->value("type", QLatin1String("unknown")).toString();
	d->trusted_devices->endGroup();

	return info;
}

void KdeConnectConfig::removeTrustedDevice(const QString &deviceId)
{
	d->trusted_devices->remove(deviceId);
	d->trusted_devices->sync();
	// We do not remove the config files
}

void KdeConnectConfig::setDeviceProperty(QString deviceId, QString key, QString value)
{
	d->trusted_devices->beginGroup(deviceId);
	d->trusted_devices->setValue(key, value);
	d->trusted_devices->endGroup();
	d->trusted_devices->sync();
}

QString KdeConnectConfig::getDeviceProperty(QString deviceId, QString key, QString defaultValue)
{
	QString value;
	d->trusted_devices->beginGroup(deviceId);
	value = d->trusted_devices->value(key, defaultValue).toString();
	d->trusted_devices->endGroup();
	return value;
}

QString KdeConnectConfig::getQcaInfo() {
	QString msg = "QCA Diagnostic:\n" + QCA::pluginDiagnosticText();
	msg += "QCA capabilities:\n" + QCA::supportedFeatures().join(", ");
	msg += "\n";
	msg += d->privateKey.toPEM().toLatin1();
	msg += d->privateKey.toPublicKey().toPEM().toLatin1();
	msg += d->certificate.toPem();
	msg += "\n";
	if(d->privateKey.canDecrypt())
		msg += "privateKey canDecrypt ";
	if(d->privateKey.canEncrypt())
		msg += "canEncrypt ";
	if(d->privateKey.canExport())
		msg += "canExport ";
	if(d->privateKey.canSign())
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
	//QSettings config;
	//config.sync();

	// base configuration directory without filename
	QFileInfo info(d->config->fileName());
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

