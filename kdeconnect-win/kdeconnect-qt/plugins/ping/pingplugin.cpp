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

#include "pingplugin.h"

#include <QDebug>

#include "core/device.h"
#include "core/kclogger.h"
#include "core/networkpackage.h"

/*
bool PingPlugin::receivePackage(const NetworkPackage& np)
{
/*
	KNotification* notification = new KNotification("pingReceived"); //KNotification::Persistent
    notification->setIconName(QStringLiteral("dialog-ok"));
    notification->setComponentName("kdeconnect");
    notification->setTitle(device()->name());
    notification->setText(np.get<QString>("message",i18n("Ping!"))); //This can be a source of spam
    notification->sendEvent();
*/
//	qDebug() << "pingplugin" << device()->name() << np.get<QString>("message",tr("Ping!"));
//    return true;
//}

void PingPlugin::sendPing() const
{
    //NetworkPackage np(PACKAGE_TYPE_PING);
    //bool success = sendPackage(np);
	//qDebug() << "sendPing:" << success;
}

void PingPlugin::sendPing(const QString& customMessage) const
{
    //NetworkPackage np(PACKAGE_TYPE_PING);
    if (!customMessage.isEmpty()) {
        //np.set("message", customMessage);
    }
    //bool success = sendPackage(np);
	//qDebug() << "sendPing:" << success;
}

void PingPlugin::initialize(const Device *device) {
	m_device = const_cast<Device*>(device);
}

void PingPlugin::connected()
{
	//TODO:QDBusConnection::sessionBus().registerObject(dbusPath(), this, QDBusConnection::ExportAllContents);
}

QString PingPlugin::info(const QString& name) {
	QString info = "PingPlugin for: " + m_device->name();
	return info;
}
