#ifndef KDECONNECTCONFIG_H
#define KDECONNECTCONFIG_H

#include <QObject>
#include <QString>
#include <QDir>
#include <QtCrypto>
#include <QSslCertificate>

// TODO:
//class QSslCertificate;
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

	QCA::PrivateKey privateKey;
	QString privateKeyPath();
	QCA::PrivateKey getPrivateKey();
	QCA::PublicKey publicKey;
	QCA::PublicKey getPublicKey();
	QString certificatePath();
	QSslCertificate certificate;
	QSslCertificate getCertificate();

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

public slots:
	QString getQcaInfo();

signals:
	void logMe(QtMsgType type, const QString &msg);
	void nameChanged(QString &name);

private:
	struct KdeConnectConfigPrivate* d;
	QCA::Initializer mQcaInitializer;	// Note it's not being used anywhere. That's intended
};

#endif // KDECONNECTCONFIG_H
