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
#include <QNetworkAccessManager>
#include <QDebug>
#include <QPointer>
#include <QMessageBox>

#include "kclogger.h"
#include "kdeconnectconfig.h"
#include "networkpackage.h"
#include "backends/lan/lanlinkprovider.h"
#include "backends/loopback/loopbacklinkprovider.h"
#include "device.h"
#include "backends/devicelink.h"
#include "backends/linkprovider.h"

Q_GLOBAL_STATIC(Daemon*, s_instance)

struct DaemonPrivate
{
	//Different ways to find devices and connect to them
	QSet<LinkProvider*> mLinkProviders;

	//Every known device
	QMap<QString, Device*> mDevices;

	QSet<QString> mDiscoveryModeAcquisitions;
};

Daemon* Daemon::instance()
{
	Q_ASSERT(s_instance.exists());
	return *s_instance;
}

Daemon::Daemon(QObject *parent, bool testMode)
	: QObject(parent)
	, d(new DaemonPrivate)
{

	KdeConnectConfig* config = new KdeConnectConfig();

	Q_ASSERT(!s_instance.exists());
	*s_instance = this;
	qDebug() << "KdeConnect daemon starting";
	Q_EMIT logMe(QtMsgType::QtInfoMsg, "Daemon  :", "WinConnect daemon starting.");

	//Load backends
	if (testMode)
		d->mLinkProviders.insert(new LoopbackLinkProvider());
	else
		d->mLinkProviders.insert(new LanLinkProvider());

	//Read remebered paired devices
	const QStringList& list = config->trustedDevices();
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

	//Register on DBus
	//QDBusConnection::sessionBus().registerService("org.kde.kdeconnect");
	//QDBusConnection::sessionBus().registerObject("/modules/kdeconnect", this, QDBusConnection::ExportScriptableContents);

	qDebug() << "KdeConnect daemon started";
	Q_EMIT logMe(QtMsgType::QtInfoMsg, "Daemon  :", "WinConnect daemon started.");
}

void Daemon::acquireDiscoveryMode(const QString &key)
{
	bool oldState = d->mDiscoveryModeAcquisitions.isEmpty();

	d->mDiscoveryModeAcquisitions.insert(key);

	if (oldState != d->mDiscoveryModeAcquisitions.isEmpty()) {
		forceOnNetworkChange();
	}
}

void Daemon::releaseDiscoveryMode(const QString &key)
{
	bool oldState = d->mDiscoveryModeAcquisitions.isEmpty();

	d->mDiscoveryModeAcquisitions.remove(key);

	if (oldState != d->mDiscoveryModeAcquisitions.isEmpty()) {
		cleanDevices();
	}
}

void Daemon::removeDevice(Device* device)
{
	d->mDevices.remove(device->id());
	device->deleteLater();
	Q_EMIT deviceRemoved(device->id());
}

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

void Daemon::forceOnNetworkChange()
{
	QString msg = "Sending onNetworkChange to " + QString::number(d->mLinkProviders.size()) + " LinkProviders";
	qDebug() << "Sending onNetworkChange to " << d->mLinkProviders.size() << " LinkProviders";
	Q_EMIT logMe(QtMsgType::QtInfoMsg, "Daemon  :", msg);
	Q_FOREACH (LinkProvider* a, d->mLinkProviders) {
		a->onNetworkChange();
	}
}

Device*Daemon::getDevice(const QString& deviceId)
{
	Q_FOREACH (Device* device, d->mDevices) {
		if (device->id() == deviceId) {
			return device;
		}
	}
	return Q_NULLPTR;
}

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

void Daemon::onNewDeviceLink(const NetworkPackage& identityPackage, DeviceLink* dl)
{
	const QString& id = identityPackage.get<QString>("deviceId");

	QString msg = "Device discovered " + id + " via " + dl->provider()->name();
	qDebug() << "Device discovered" << id << "via" << dl->provider()->name();
	Q_EMIT logMe(QtMsgType::QtInfoMsg, "Daemon  :", msg);

	if (d->mDevices.contains(id)) {
		qDebug() << "It is a known device" << identityPackage.get<QString>("deviceName");
		QString msg = "It is a known device " + identityPackage.get<QString>("deviceName");
		Q_EMIT logMe(QtMsgType::QtInfoMsg, "Daemon  :", msg);
		Device* device = d->mDevices[id];
		bool wasReachable = device->isReachable();
		device->addLink(identityPackage, dl);
		if (!wasReachable) {
			Q_EMIT deviceVisibilityChanged(id, true);
		}
	} else {
		qDebug() << "It is a new device" << identityPackage.get<QString>("deviceName");
		QString msg = "It is a new device " + identityPackage.get<QString>("deviceName");
		Q_EMIT logMe(QtMsgType::QtInfoMsg, "Daemon  :", msg);
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

void Daemon::onDeviceStatusChanged()
{
	Device* device = (Device*)sender();

	qDebug() << "Device" << device->name() << "status changed. Reachable:" << device->isReachable();// << ". Paired: " << device->isPaired();
	QString msg = "Device " + device->name() + " status changed. Reachable: " + device->isReachable();
	Q_EMIT logMe(QtMsgType::QtInfoMsg, "Daemon  :", msg);

	if (!device->isReachable() && !device->isTrusted()) {
		qDebug() << "Destroying device" << device->name();
		QString msg = "Destroying device " + device->name();
		Q_EMIT logMe(QtMsgType::QtInfoMsg, "Daemon  :", msg);
		removeDevice(device);
	} else {
		Q_EMIT deviceVisibilityChanged(device->id(), device->isReachable());
	}

}

void Daemon::setAnnouncedName(const QString &name)
{
	qDebug() << "Announcing name";
	QString msg = "Announcing name: " + name;
	Q_EMIT logMe(QtMsgType::QtInfoMsg, "Daemon  :", msg);
	KdeConnectConfig* config = new KdeConnectConfig();
	config->setName(name);
	forceOnNetworkChange();
	Q_EMIT announcedNameChanged(name);
}

QString Daemon::announcedName()
{
	KdeConnectConfig* config = new KdeConnectConfig();
	return config->name();
}

QNetworkAccessManager* Daemon::networkAccessManager()
{
	static QPointer<QNetworkAccessManager> manager;
	if (!manager) {
		manager = new QNetworkAccessManager(this);
	}
	return manager;
}

QList<Device*> Daemon::devicesList() const
{
	return d->mDevices.values();
}

bool Daemon::isDiscoveringDevices() const
{
	return !d->mDiscoveryModeAcquisitions.isEmpty();
}

QString Daemon::deviceIdByName(const QString &name) const
{
	Q_FOREACH (Device* d, d->mDevices) {
		if (d->name() == name && d->isTrusted())
			return d->id();
	}
	return {};
}

void Daemon::askPairingConfirmation(PairingHandler *d)
{
	qDebug() << "askPairingConfirmation()";
	Q_EMIT logMe(QtMsgType::QtInfoMsg, "Daemon  :", "Asking for pairing confirmation");

	QMessageBox msgBox;
	msgBox.setText("Pairing request");
	msgBox.setInformativeText("Accept pairing?");
	msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Discard);
	msgBox.setDefaultButton(QMessageBox::Ok);
	int ret = msgBox.exec();
	switch(ret) {
		case QMessageBox::Ok:
			d->acceptPairing();
			break;
		case QMessageBox::Discard:
			d->rejectPairing();
			break;
	}
}

void Daemon::reportError(const QString &title, const QString &description)
{
	qDebug() << "Daemon: " << title << description;
	QString msg = title + " " + description;
	Q_EMIT logMe(QtMsgType::QtDebugMsg, "Daemon  :", msg);
}

Daemon::~Daemon()
{

}
