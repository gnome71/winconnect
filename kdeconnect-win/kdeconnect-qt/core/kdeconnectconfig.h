#ifndef KDECONNECTCONFIG_H
#define KDECONNECTCONFIG_H

#include <QObject>
#include <QDir>

class QsslCertificate;
namespace QCA {
    class PrivateKey;
    class PublicKey;
}

//! KdeConnectConfig class definition
class KdeConnectConfig
	: public QObject
{
	Q_OBJECT

public:
    KdeConnectConfig();

    struct DeviceInfo {
        QString deviceName;
        QString deviceType;
    };

    // our own info
    QString deviceId();
    QString name();
    QString deviceType();

    QString privateKeyPath();
    QCA::PrivateKey privateKey();
    QCA::PublicKey publicKey();
    QString certificatePath();
//    QsslCertificate certificate();

    void setName(QString name);

    // trusted devices
    QStringList trustedDevices(); //list of ids
    void removeTrustedDevice(const QString &id);
    void addTrustedDevice(const QString &id, const QString &name, const QString &type);
    KdeConnectConfig::DeviceInfo getTrustedDevice(const QString &id);

    void setDeviceProperty(QString deviceId, QString name, QString value);
    QString getDeviceProperty(QString deviceId, QString name, QString defaultValue = QString());


    // Paths for config files, there is no guarantee the directories already exist
    QDir baseConfigDir();
    QDir deviceConfigDir(const QString &deviceId);
    QDir pluginConfigDir(const QString &deviceId, const QString &pluginName); //Used by KdeConnectPluginConfig

signals:
	void logMe(QtMsgType type, const QString &msg);
 
private:
    struct KdeConnectConfigPrivate* d;
};

#endif // KDECONNECTCONFIG_H
