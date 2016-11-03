/**
 * Copyright 2013 Albert Vaca <albertvaka@gmail.com>
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

#include "devicesmodel.h"

#include <QCoreApplication>
#include <QString>
#include <QIcon>
#include <QDebug>


static QString createId() { return QCoreApplication::instance()->applicationName()+QString::number(QCoreApplication::applicationPid()); }
Q_GLOBAL_STATIC_WITH_ARGS(QString, s_keyId, (createId()));

DevicesModel::DevicesModel(QObject *parent)
    : QAbstractListModel(parent)
	, m_daemonInterface(Daemon::instance())
    , m_displayFilter(StatusFilterFlag::NoFilter)
{

    //new ModelTest(this, this);

    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SIGNAL(rowsChanged()));
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SIGNAL(rowsChanged()));

	connect(m_daemonInterface, SIGNAL(deviceAdded(QString)),
            this, SLOT(deviceAdded(QString)));
	connect(m_daemonInterface, SIGNAL(deviceVisibilityChanged(QString,bool)),
            this, SLOT(deviceUpdated(QString,bool)));
	connect(m_daemonInterface, SIGNAL(deviceRemoved(QString)),
            this, SLOT(deviceRemoved(QString)));

    //refresh the view, acquireDiscoveryMode if necessary
    setDisplayFilter(NoFilter);
}

/**
 * @brief DevicesModel::roleNames
 * @return
 */
QHash< int, QByteArray > DevicesModel::roleNames() const
{
    QHash<int, QByteArray> names = QAbstractItemModel::roleNames();
    names.insert(IdModelRole, "deviceId");
    names.insert(IconNameRole, "iconName");
    names.insert(DeviceRole, "device");
//    names.insert(StatusModelRole, "status");
    return names;
}

DevicesModel::~DevicesModel()
{
	m_daemonInterface->releaseDiscoveryMode(*s_keyId);
}

int DevicesModel::rowForDevice(const QString& id) const
{
    for (int i = 0, c=m_deviceList.size(); i<c; ++i) {
		qDebug() << m_daemonInterface->devicesList().at(i)->id();
		if (m_daemonInterface->devicesList().at(i)->id() == id) {
            return i;
        }
    }
    return -1;
}

void DevicesModel::deviceAdded(const QString& id)
{
    if (rowForDevice(id) >= 0) {
        Q_ASSERT_X(false, "deviceAdded", "Trying to add a device twice");
        return;
    }

//    DeviceDbusInterface* dev = new DeviceDbusInterface(id, this);
//    Q_ASSERT(dev->isValid());

//    if (! passesFilter(dev)) {
//        delete dev;
//        return;
//    }

    beginInsertRows(QModelIndex(), m_deviceList.size(), m_deviceList.size());
//    appendDevice(dev);
    endInsertRows();
}

void DevicesModel::deviceRemoved(const QString& id)
{
    int row = rowForDevice(id);
    if (row>=0) {
        beginRemoveRows(QModelIndex(), row, row);
		m_deviceList.removeAt(row);
        endRemoveRows();
    }
}

void DevicesModel::deviceUpdated(const QString& id, bool isVisible)
{
    Q_UNUSED(isVisible);
    int row = rowForDevice(id);

    if (row < 0) {
        // FIXME: when m_dbusInterface is not valid refreshDeviceList() does
        // nothing and we can miss some devices.
        // Someone can reproduce this problem by restarting kdeconnectd while
        // kdeconnect's plasmoid is still running.
        // Another reason for this branch is that we removed the device previously
        // because of the filter settings.
		qDebug() << "Adding missing or previously removed device" << id;
        deviceAdded(id);
    } else {
		Device* dev = getDevice(row);
        if (! passesFilter(dev)) {
            beginRemoveRows(QModelIndex(), row, row);
			m_deviceList.removeAt(row);
            endRemoveRows();
			qDebug() << "Removed changed device " << id;
        } else {
            const QModelIndex idx = index(row);
            Q_EMIT dataChanged(idx, idx);
        }
    }
}

Device* DevicesModel::getDevice(int row) const
{
	QList<Device*> devs = m_daemonInterface->devicesList();
	Device* dev = devs.at(row);
}

int DevicesModel::displayFilter() const
{
    return m_displayFilter;
}

void DevicesModel::setDisplayFilter(int flags)
{
    m_displayFilter = (StatusFilterFlag)flags;

    const bool reachableNeeded = (m_displayFilter & StatusFilterFlag::Reachable);
    if (reachableNeeded)
		m_daemonInterface->acquireDiscoveryMode(*s_keyId);
    else
		m_daemonInterface->releaseDiscoveryMode(*s_keyId);

    refreshDeviceList();
}

void DevicesModel::refreshDeviceList()
{
//	if (!m_daemonInterface->isValid()) {
//        clearDevices();
//		qWarning() << "daemon interface not valid";
//        return;
//    }

    bool onlyPaired = (m_displayFilter & StatusFilterFlag::Paired);
    bool onlyReachable = (m_displayFilter & StatusFilterFlag::Reachable);

//    QDBusPendingReply<QStringList> pendingDeviceIds = m_dbusInterface->devices(onlyReachable, onlyPaired);
//    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingDeviceIds, this);

//    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
//                     this, SLOT(receivedDeviceList(QDBusPendingCallWatcher*)));
}

/*
void DevicesModel::receivedDeviceList(QDBusPendingCallWatcher* watcher)
{
    watcher->deleteLater();
    clearDevices();
    QDBusPendingReply<QStringList> pendingDeviceIds = *watcher;
    if (pendingDeviceIds.isError()) {
        qCWarning(KDECONNECT_INTERFACES) << "error while refreshing device list" << pendingDeviceIds.error().message();
        return;
    }

    Q_ASSERT(m_deviceList.isEmpty());
    const QStringList deviceIds = pendingDeviceIds.value();

    if (deviceIds.isEmpty())
        return;

    beginInsertRows(QModelIndex(), 0, deviceIds.count()-1);
    Q_FOREACH(const QString& id, deviceIds) {
        appendDevice(new DeviceDbusInterface(id, this));
    }
    endInsertRows();
}
*/
void DevicesModel::appendDevice(Device* dev)
{
	m_deviceList.append(dev->name());
    connect(dev, SIGNAL(nameChanged(QString)), SLOT(nameChanged(QString)));
}

void DevicesModel::nameChanged(const QString& newName)
{
    Q_UNUSED(newName);
	Device* device = static_cast<Device*>(sender());

    Q_ASSERT(rowForDevice(device->id()) >= 0);

    deviceUpdated(device->id(), true);
}

void DevicesModel::clearDevices()
{
    if (!m_deviceList.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_deviceList.size() - 1);
        qDeleteAll(m_deviceList);
        m_deviceList.clear();
        endRemoveRows();
    }
}

QVariant DevicesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()
        || index.row() < 0
        || index.row() >= m_deviceList.size())
    {
        return QVariant();
    }

	Q_ASSERT(m_daemonInterface == Q_NULLPTR);

	QString device = m_deviceList[index.row()];
	Q_ASSERT(device == Q_NULLPTR);

    //This function gets called lots of times, producing lots of dbus calls. Add a cache?
    switch (role) {
        case IconModelRole: {
            QString icon = data(index, IconNameRole).toString();
            return QIcon::fromTheme(icon);
        }
        case IdModelRole:
            return device->id();
        case NameModelRole:
            return device->name();
        case Qt::ToolTipRole: {
            bool trusted = device->isTrusted();
            bool reachable = device->isReachable();
            QString status = reachable? (trusted? i18n("Device trusted and connected") : i18n("Device not trusted")) : i18n("Device disconnected");
            return status;
        }
        case StatusModelRole: {
            int status = StatusFilterFlag::NoFilter;
            if (device->isReachable()) {
                status |= StatusFilterFlag::Reachable;
            }
            if (device->isTrusted()) {
                status |= StatusFilterFlag::Paired;
            }
            return status;
        }
        case IconNameRole:
            return device->statusIconName();
        case DeviceRole:
            return QVariant::fromValue<QObject*>(device);
        default:
            return QVariant();
    }
}

DeviceDbusInterface* DevicesModel::getDevice(int row) const
{
    if (row < 0 || row >= m_deviceList.size()) {
        return nullptr;
    }

    return m_deviceList[row];
}

int DevicesModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid()) {
        //Return size 0 if we are a child because this is not a tree
        return 0;
    }

    return m_deviceList.size();
}

bool DevicesModel::passesFilter(Device* dev) const
{
    bool onlyPaired = (m_displayFilter & StatusFilterFlag::Paired);
    bool onlyReachable = (m_displayFilter & StatusFilterFlag::Reachable);

    return !((onlyReachable && !dev->isReachable()) || (onlyPaired && !dev->isTrusted()));
}
