#include "kdeconnectconfig.h"
#include "kclogger.h"
#include "deviceidhelper.h"
#include "daemon.h"

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

KdeConnectConfig* KdeConnectConfig::instance()
{
	static KdeConnectConfig* kcc = new KdeConnectConfig();
	return kcc;
}

KdeConnectConfig::KdeConnectConfig()
	: d(new KdeConnectConfigPrivate)
{

	if(!QCA::isSupported("rsa")) {
		qDebug() << "RSA not supported";
		return;
	}
	else if(!QCA::isSupported("pkey") ||
			!QCA::PKey::supportedIOTypes().contains(QCA::PKey::RSA)) {
		qDebug() << "PKEY not supported";
		return;
	}

	d->config = new QSettings(baseConfigDir().absoluteFilePath("config"), QSettings::IniFormat);
	d->trusted_devices = new QSettings(baseConfigDir().absoluteFilePath("trusted_devices"), QSettings::IniFormat);

	//Register my own id if not there yet
	if(!d->config->contains("id")) {
		QString uuid = QUuid::createUuid().toString();
		DeviceIdHelper::filterNonExportableCharacters(uuid);
		d->config->setValue("id", uuid);
		d->config->sync();
		qDebug() << "My id:" << uuid;
		KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "My id: " + uuid);
	}

	// Load or register new private key if not there
	QString keyPath = privateKeyPath();
	QFile privKey(keyPath);
	QCA::PrivateKey privateKey;
	if(privKey.exists() && privKey.open(QIODevice::ReadOnly)) {
		d->privateKey = QCA::PrivateKey::fromPEM(privKey.readAll());
		//qDebug() << "Opened private key: " << keyPath;
		if(d->privateKey.isNull())
			qDebug() << "load: privateKey.isNull";
		privKey.close();
	}
	else {
		d->privateKey = QCA::KeyGenerator().createRSA(2048);
		if(!privKey.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
			qDebug() << "Could not store private key file: " << privKey.fileName();
		}
		else {
			int len = privKey.write(d->privateKey.toPEM().toLatin1());
			qDebug() << "Created private key with length: " << len;
			KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "PrivateKey created. Length: " + QString::number(len));
			privKey.close();
		}
	}

	// Load or register certificate if not there
	QString certPath = certificatePath();
	QFile cert(certPath);
	QSslCertificate certificate;
	if(cert.exists() && cert.open(QIODevice::ReadOnly)) {
		if(!QSslCertificate::fromPath(certPath, QSsl::Pem).at(0).isNull()) {
			d->certificate = QSslCertificate::fromPath(certPath).at(0);
			//qDebug() << "Opened Cert: " << certPath;
		}
		else { qDebug() << "Unable to open: " + certPath; }

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

		d->certificate = QSslCertificate(QCA::Certificate(certificateOptions, d->privateKey).toPEM().toLatin1());

		if(!cert.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
			qDebug() << "Could not store certificate file: " + cert.fileName();
		}
		else {
			cert.write(d->certificate.toPem());
			cert.close();
		}
	}
}

QString KdeConnectConfig::deviceId()
{
	QString ret = d->config->value("id", "").toString();
	return ret;
}

QString KdeConnectConfig::name()
{
	QString defaultName = qgetenv("USERNAME") + '@' + QHostInfo::localHostName();
	QString name =  d->config->value("name", defaultName).toString();
	return name;
}

QString KdeConnectConfig::deviceType()
{
	return "desktop";
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
	d->config->setValue("name", name);
	d->trusted_devices->sync();
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
	d->trusted_devices->setValue("paired", true);
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
	QCA::PrivateKey pk = d->privateKey;
	QSslCertificate cert = d->certificate;
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
	// base configuration directory without filename
	QString configPath = QStandardPaths::writableLocation(QStandardPaths:: AppDataLocation);
//	QString kdeconnectConfigPath = QDir(configPath);
	return QDir(configPath);
}

/**
 * @brief KdeConnectConfig::deviceConfigDir
 * returns also the base path (using one config file)
 * @param deviceId
 * @return QDir
 */
QDir KdeConnectConfig::deviceConfigDir(const QString &deviceId)
{
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

