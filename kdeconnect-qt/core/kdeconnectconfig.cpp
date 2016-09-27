#include "kdeconnectconfig.h"

#include <QFile>
#include <QDebug>
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
    //qCDebug(KDECONNECT_CORE) << "QCA supported capabilities:" << QCA::supportedFeatures().join(",");
    if(!QCA::isSupported("rsa")) {
        Daemon::instance()->reportError(
                             i18n("KDE Connect failed to start"),
                             i18n("Could not find support for RSA in your QCA installation. If your "
                                  "distribution provides separate packages for QCA-ossl and QCA-gnupg, "
                                  "make sure you have them installed and try again."));
        return;
  }
}

QString KdeConnectConfig::deviceId()
{

}

QString KdeConnectConfig::name()
{

}

QString KdeConnectConfig::deviceType()
{

}

QString KdeConnectConfig::privateKeyPath()
{

}

QCA::PrivateKey KdeConnectConfig::privateKey()
{

}

QCA::PublicKey KdeConnectConfig::publicKey()
{

}

QString KdeConnectConfig::certificatePath()
{

}

QsslCertificate KdeConnectConfig::certificate()
{

}

void KdeConnectConfig::setName(QString name)
{

}

QStringList KdeConnectConfig::trustedDevices()
{

}

void KdeConnectConfig::removeTrustedDevice(const QString &id)
{

}

void KdeConnectConfig::addTrustedDevice(const QString &id, const QString &name, const QString &type)
{

}

KdeConnectConfig::DeviceInfo KdeConnectConfig::getTrustedDevice(const QString &id)
{

}

void KdeConnectConfig::setDeviceProperty(QString deviceId, QString name, QString value)
{

}

QString KdeConnectConfig::getDeviceProperty(QString deviceId, QString name, QString defaultValue)
{

}

QDir KdeConnectConfig::baseConfigDir()
{

}

QDir KdeConnectConfig::deviceConfigDir(const QString &deviceId)
{

}

QDir KdeConnectConfig::pluginConfigDir(const QString &deviceId, const QString &pluginName)
{

}
