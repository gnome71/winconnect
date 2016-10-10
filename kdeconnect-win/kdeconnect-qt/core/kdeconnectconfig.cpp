#include "kdeconnectconfig.h"
#include "core/kclogger.h"

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

	QCA::PrivateKey privateKey;
	QSslCertificate certificate; // Use QSslCertificate instead of QCA::Certificate due to compatibility with QSslSocket

	QSettings* config;
	QSettings* trusted_devices;
};

KdeConnectConfig::KdeConnectConfig()
{
	//QLoggingCategory kcQca("kc.qca");

	QCA::Initializer mQcaInitializer;

	if(!QCA::isSupported("rsa")) {
		emit logMe(QtMsgType::QtCriticalMsg, "RSA not supported");
		return;
	}

	QSettings::setDefaultFormat(QSettings::IniFormat);

	QSettings config;
	config.sync();

	//Register my own id if not there yet
	if (!config.contains("my/id")) {
		QString uuid = QUuid::createUuid().toString();
		config.setValue("my/id", uuid);
		config.sync();
		qCDebug(kcQca) << "My id:" << uuid;
	}

}

QString KdeConnectConfig::deviceId()
{
	return "todo";
}

QString KdeConnectConfig::name()
{
	return "todo";
}

QString KdeConnectConfig::deviceType()
{
	return "todo";
}

QString KdeConnectConfig::privateKeyPath()
{

	return "todo";
}

QCA::PrivateKey KdeConnectConfig::privateKey()
{
	qCDebug(kcQca) << "privateKey()";
	QCA::PrivateKey privatekey;
	return privatekey;
}

QCA::PublicKey KdeConnectConfig::publicKey()
{
	QCA::PublicKey pubkey;
	return pubkey;
}

QString KdeConnectConfig::certificatePath()
{
	return "todo";
}

//QsslCertificate KdeConnectConfig::certificate()
//{
//
//}

void KdeConnectConfig::setName(QString name)
{

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

	// base configuration directory
	qDebug() << config.fileName();
	QDir bcd(config.fileName());

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

