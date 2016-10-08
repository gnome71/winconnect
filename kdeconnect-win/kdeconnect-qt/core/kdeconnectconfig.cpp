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

    // The Initializer object sets things up, and also does cleanup when it goes out of scope
    // Note it's not being used anywhere. That's intended
    QCA::Initializer mQcaInitializer;

    QCA::PrivateKey privateKey;
    QSslCertificate certificate; // Use QSslCertificate instead of QCA::Certificate due to compatibility with QSslSocket

    QSettings* config;
    QSettings* trusted_devices;

};

KdeConnectConfig::KdeConnectConfig()
{
	//QLoggingCategory kcQca("kc.qca");

	QCA::Initializer mQcaInitializer;

    QCA::scanForPlugins();

    qCDebug(kcQca) << "QCA Diagnostic: " << QCA::pluginDiagnosticText();

	emit logMe(QtMsgType::QtDebugMsg, "QCA supported capabilities: "
		+ QCA::supportedFeatures().join(","));

	qCDebug(kcQca) << "QCA supported capabilities: " 
		<< QCA::supportedFeatures().join(",");

	if(!QCA::isSupported("rsa")) {
		qCDebug(kcQca) << "RSA not supported";
        return;
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

QDir KdeConnectConfig::baseConfigDir()
{
    return QDir("todo");
}

QDir KdeConnectConfig::deviceConfigDir(const QString &deviceId)
{
    return QDir("todo");
}

QDir KdeConnectConfig::pluginConfigDir(const QString &deviceId, const QString &pluginName)
{
    return QDir("todo");
}

