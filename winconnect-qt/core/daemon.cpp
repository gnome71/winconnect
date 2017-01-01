/**
 * Copyright 2014 Albert Vaca <albertvaka@gmail.com>
 * Modified for Windows by Alexander Kaspar <alexander.kaspar@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "daemon.h"

//#include <QDBusConnection>
#include <QApplication>
#include <QNetworkAccessManager>
#include <QDebug>
#include <QDir>
#include <QObject>
#include <QPointer>
#include <QMessageBox>
#include <QPluginLoader>
#include <QLibraryInfo>

#include "kclogger.h"
#include "kdeconnectconfig.h"
#include "device.h"
#include "networkpackage.h"
#include "backends/lan/lanlinkprovider.h"
//#include "backends/loopback/loopbacklinkprovider.h"
#include "backends/devicelink.h"
#include "backends/linkprovider.h"
#include "interfaces/notificationinterface.h"

Q_GLOBAL_STATIC(Daemon*, s_instance)

/**
 * @brief The DaemonPrivate struct
 */
struct DaemonPrivate
{
	QSet<LinkProvider*> mLinkProviders;	//! Different ways to find devices and connect to them
	QMap<QString, Device*> mDevices;	//! Every known device
	QSet<QString> mDiscoveryModeAcquisitions;
};

/**
 * @brief Daemon::instance
 * @return Q_GLOBAL_STATIC pointer to Daemon
 */
Daemon* Daemon::instance()
{
	Q_ASSERT(s_instance.exists());
	return *s_instance;
}

/**
 * @brief Daemon::Daemon
 * @param parent
 * @param testMode, Lan or Loopback backend
 */
Daemon::Daemon(QObject *parent, bool testMode)
	: QObject(parent)
	, d(new DaemonPrivate)
{
	Q_ASSERT(!s_instance.exists());
	*s_instance = this;
	qDebug() << "KdeConnect daemon starting";
	KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "WinConnect daemon starting.");

	// Load plugins
	loadPlugins();

	//Load backends
	if (testMode);
		//d->mLinkProviders.insert(new LoopbackLinkProvider());
	else
		d->mLinkProviders.insert(new LanLinkProvider());

	//Read remebered paired devices
	const QStringList& list = KdeConnectConfig::instance()->trustedDevices();
	Q_FOREACH (const QString& id, list) {
		Device* device = new Device(this, id);
		connect(device, SIGNAL(reachableStatusChanged()), this, SLOT(onDeviceStatusChanged()));
		connect(device, SIGNAL(trustedChanged(bool)), this, SLOT(onDeviceStatusChanged()));
		d->mDevices[id] = device;
		Q_EMIT deviceAdded(id);
	}

	//Listen to new devices
	Q_FOREACH (LinkProvider* a, d->mLinkProviders) {
		connect(a, SIGNAL(onConnectionReceived(NetworkPackage,DeviceLink*)),
				this, SLOT(onNewDeviceLink(NetworkPackage,DeviceLink*)));
		a->onStart();
	}

	qDebug() << "KdeConnect daemon started";
	KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "WinConnect daemon started.");
}

void Daemon::loadPlugins()
{
	QDir libpath = qApp->applicationDirPath();		//QLibraryInfo::location(QLibraryInfo::PluginsPath);
	libpath.cd("plugins");
	QStringList filters;
	filters << "*.dll";
	libpath.setNameFilters(filters);
	foreach (QString fileName, libpath.entryList(filters)) {
		QString ld = libpath.absoluteFilePath(fileName);
		QPluginLoader loader(libpath.absoluteFilePath(fileName));
		QObject *plugin = loader.instance();
		if (plugin) {
			pluginFileNames += fileName;
			QJsonValue pluginMetadata(loader.metaData().value("MetaData"));
			QJsonObject metaDataObject = pluginMetadata.toObject();
			qDebug() << "PluginName: " << metaDataObject.value("name").toString();
			qDebug() << "PluginVer.: " << metaDataObject.value("version").toString();
			QString pName = metaDataObject.value("name").toString();
			if (pName == "BatteryPlugin") {
				batteryPlugin = qobject_cast<BatteryPluginInterface*>(plugin);
				if (batteryPlugin) {
					qDebug() << "From batteryPlugin: " << batteryPlugin->charge();
				}
			}
			else if (pName == "PingPlugin") {
				pingPlugin = qobject_cast<PingPluginInterface*>(plugin);
				if (pingPlugin) {
					qDebug() << "From pingPlugin: " << pingPlugin->info(fileName);
				}
			}
			else if (pName == "TestPluginA") {
				testPlugin = qobject_cast<PluginInterface*>(plugin);
				if (testPlugin) {
					qDebug() << "From testPlugin: " << testPlugin->info(fileName);
				}
			}
		}
	}

	qDebug() << "Plugins:" << pluginFileNames;
}

/**
 * @brief Daemon::acquireDiscoveryMode
 * @param key
 */
void Daemon::acquireDiscoveryMode(const QString &key)
{
	bool oldState = d->mDiscoveryModeAcquisitions.isEmpty();

	d->mDiscoveryModeAcquisitions.insert(key);

	if (oldState != d->mDiscoveryModeAcquisitions.isEmpty()) {
		forceOnNetworkChange();
	}
}

/**
 * @brief Daemon::releaseDiscoveryMode
 * @param key
 */
void Daemon::releaseDiscoveryMode(const QString &key)
{
	bool oldState = d->mDiscoveryModeAcquisitions.isEmpty();

	d->mDiscoveryModeAcquisitions.remove(key);

	if (oldState != d->mDiscoveryModeAcquisitions.isEmpty()) {
		cleanDevices();
	}
}

/**
 * @brief Daemon::removeDevice
 * @param device
 */
void Daemon::removeDevice(Device* device)
{
	d->mDevices.remove(device->id());
	device->deleteLater();
	Q_EMIT deviceRemoved(device->id());
}

/**
 * @brief Daemon::cleanDevices
 */
void Daemon::cleanDevices()
{
	Q_FOREACH (Device* device, d->mDevices) {
		if (device->isTrusted()) {
			continue;
		}
		device->cleanUnneededLinks();
		//If there are no links remaining
		if (!device->isReachable()) {
			removeDevice(device);
		}
	}
}

/**
 * @brief Daemon::forceOnNetworkChange
 */
void Daemon::forceOnNetworkChange()
{
	qDebug() << "Sending onNetworkChange to " << d->mLinkProviders.size() << " LinkProviders";
	KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "Sending onNetworkChange to " + QString::number(d->mLinkProviders.size()) + " LinkProviders");
	Q_FOREACH (LinkProvider* a, d->mLinkProviders) {
		a->onNetworkChange();
	}
}

/**
 * @brief Daemon::devices
 * @param onlyReachable
 * @param onlyTrusted
 * @return QStringList, list of deviceIds
 */
QStringList Daemon::devices(bool onlyReachable, bool onlyTrusted) const
{
	QStringList ret;
	Q_FOREACH (Device* device, d->mDevices) {
		if (onlyReachable && !device->isReachable()) continue;
		if (onlyTrusted && !device->isTrusted()) continue;
		ret.append(device->id());
	}
	return ret;
}

/**
 * @brief Daemon::onNewDeviceLink
 * @param identityPackage
 * @param dl
 */
void Daemon::onNewDeviceLink(const NetworkPackage& identityPackage, DeviceLink* dl)
{
	const QString& id = identityPackage.get<QString>("deviceId");

	qDebug() << "Device discovered" << id << "via" << dl->provider()->name();
	KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "Device discovered " + id + " via " + dl->provider()->name());

	if (d->mDevices.contains(id)) {
		qDebug() << "It is a known device" << identityPackage.get<QString>("deviceName");
		KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "It is a known device " + identityPackage.get<QString>("deviceName"));
		Device* device = d->mDevices[id];
		bool wasReachable = device->isReachable();
		device->addLink(identityPackage, dl);

		// adding plugin for device
		qDebug() << device->name();
		QVariant deviceVariant = QVariant::fromValue<Device*>(device);	//TODO: Bug
		QVariantList m_args;
		m_args << deviceVariant;

		//testPlugin->sendPing(m_args);

		if (!wasReachable) {
			Q_EMIT deviceVisibilityChanged(id, true);
		}
	} else {
		qDebug() << "It is a new device" << identityPackage.get<QString>("deviceName");
		KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "It is a new device " + identityPackage.get<QString>("deviceName"));
		Device* device = new Device(this, identityPackage, dl);

		//we discard the connections that we created but it's not paired.
		if (!isDiscoveringDevices() && !device->isTrusted() && !dl->linkShouldBeKeptAlive()) {
			device->deleteLater();
		} else {
			connect(device, SIGNAL(reachableStatusChanged()), this, SLOT(onDeviceStatusChanged()));
			connect(device, SIGNAL(trustedChanged(bool)), this, SLOT(onDeviceStatusChanged()));
			d->mDevices[id] = device;

			Q_EMIT deviceAdded(id);
		}
	}
}

/**
 * @brief Daemon::onDeviceStatusChanged
 */
void Daemon::onDeviceStatusChanged()
{
	Device* device = (Device*)sender();

	QString t, r;
	device->isReachable()? r = "true" : r = "false";
	device->isTrusted()? t = "true" : t = "false";

	qDebug() << "Device" << device->name() << "status changed. Reachable:" << device->isReachable() << ", Trusted:" << device->isTrusted();
	KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "Device " + device->name() + " status changed. Reachable: " + r + " Trusted: " + t);

	if (!device->isReachable() && !device->isTrusted()) {
		qDebug() << "Destroying device" << device->name();
		KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "Destroying device " + device->name());
		removeDevice(device);
	} else {
		Q_EMIT deviceVisibilityChanged(device->id(), device->isReachable());
	}

}

/**
 * @brief Daemon::setAnnouncedName
 * @param name, the name of the local device
 */
void Daemon::setAnnouncedName(const QString &name)
{
	qDebug() << "Announcing name";
	KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "Announcing name: " + name);
	KdeConnectConfig::instance()->setName(name);
	forceOnNetworkChange();
	Q_EMIT announcedNameChanged(name);
}

/**
 * @brief Daemon::announcedName
 * @return name of the local device
 */
QString Daemon::announcedName()
{
	return KdeConnectConfig::instance()->name();
}

/**
 * @brief Daemon::networkAccessManager
 *
 * The QNetworkAccessManager class allows the application to send
 * network requests and receive replies.
 * @return
 */
QNetworkAccessManager* Daemon::networkAccessManager()
{
	static QPointer<QNetworkAccessManager> manager;
	if (!manager) {
		manager = new QNetworkAccessManager(this);
	}
	return manager;
}

/**
* @brief Daemon::getDevice
* @param deviceId
* @return
*/
Device * Daemon::getDevice(const QString & deviceId)
{
	Q_FOREACH(Device* device, d->mDevices) {
		if (device->id() == deviceId) {
			return device;
		}
	}
	return nullptr;
}

/**
 * @brief Daemon::devicesList
 * @return List of pointer to devices
 */
QList<Device*> Daemon::devicesList() const
{
	return d->mDevices.values();
}

/**
 * @brief Daemon::isDiscoveringDevices
 * @return
 */
bool Daemon::isDiscoveringDevices() const
{
	return !d->mDiscoveryModeAcquisitions.isEmpty();
}

/**
 * @brief Daemon::deviceIdByName
 * @param name
 * @return deviceId
 */
QString Daemon::deviceIdByName(const QString &name) const
{
	Q_FOREACH (Device* d, d->mDevices) {
		if (d->name() == name /*&& d->isTrusted()*/)
			return d->id();
	}
	return {};
}

/**
 * @brief Daemon::askPairingConfirmation
 *
 * Ask the user for pairing confirmation to a device
 * @param d
 */
void Daemon::askPairingConfirmation(PairingHandler *d)
{
	const QString& dev = getDevice(d->deviceLink()->deviceId())->name();
	KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "Asking user for pairing confirmation");

	Q_EMIT askPairing(dev, "Confirm Pairing by clicking this message.");

	QMessageBox msgBox;
	msgBox.setText("Pairing request from " + dev);
	msgBox.setInformativeText("Accept pairing?");
	msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Discard);
	msgBox.setDefaultButton(QMessageBox::Ok);
	int ret = msgBox.exec();
	switch(ret) {
		case QMessageBox::Ok:
			d->acceptPairing();
			break;
		case QMessageBox::Discard:
			d->rejectPairing();		//BUG: read access violation?
			break;
	}
}

/**
 * @brief Daemon::reportError
 * @param title
 * @param description
 */
void Daemon::reportError(const QString &title, const QString &description)
{
	qDebug() << "Daemon: " << title << description;
	KcLogger::instance()->write(QtMsgType::QtDebugMsg, prefix, title + " " + description);
}

/**
 * @brief Daemon::~Daemon
 */
Daemon::~Daemon()
{

}
