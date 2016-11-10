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
#ifndef NOTIFICATIONINTERFACE_H
#define NOTIFICATIONINTERFACE_H

#include <QObject>

#include "core/kclogger.h"
#include "core/device.h"

class NotificationInterface
		: public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString deviceId READ deviceId CONSTANT)
	//Q_PROPERTY(QString deviceName READ deviceName CONSTANT)

public:
	explicit NotificationInterface(QObject *parent, const QString& deviceId);

signals:
  void logMe(QtMsgType type, const QString &m_Prefix, const QString &msg);

public slots:
	QStringList notificationList();

private slots:
	QString deviceId() const { return m_deviceId; }
	//QString deviceName() const { return Device::

private:
	const QString& m_Prefix = "NotifyInt ";
	const QString& m_deviceId;
};

#endif // NOTIFICATIONINTERFACE_H
