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


#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QString>
#include <QVector>

#include "core/networkpackage.h"
#include "backends/devicelink.h"

class DeviceLink;

class Device : public QObject
{
	Q_OBJECT

public:
	enum DeviceType {
		Unknown,
		Desktop,
		Laptop,
		Phone,
		Tablet
	};

	enum TrustStatus {
		NotTrusted,
		Trusted
	};

	Device(QObject* parent, const QString& id);
	Device(QObject* parent, const NetworkPackage& np, DeviceLink* dl);
	~Device();

	QString id() const { return m_deviceId; }
	QString name() const { return m_deviceName; }
	QString type() const { return type2str(m_deviceType); }
	Q_SCRIPTABLE QString encryptionInfo();

//	void addLink(const NetworkPackage& identityPackage, DeviceLink*);
//	void removeLink(DeviceLink*);

	Q_SCRIPTABLE bool isTrusted();

	Q_SCRIPTABLE QStringList availableLinks();
//	bool isReachable() const { return !m_deviceLinks.isEmpty(); }

	//void cleanUnneededLinks();

	int protocolVersion() { return m_protocolVersion; }

public slots:
	virtual bool sendPackage(NetworkPackage& np);

signals:
	Q_SCRIPTABLE void nameChanged(const QString& name);
	Q_SCRIPTABLE void pairingError(const QString& error);

private:
	KdeConnectConfig config;
	static DeviceType str2type(const QString &deviceType);
	static QString type2str(DeviceType deviceType);

	void setName(const QString &name);

	const QString m_deviceId;
	QString m_deviceName;
	DeviceType m_deviceType;
	int m_protocolVersion;

	QVector<DeviceLink*> m_deviceLinks;
};

Q_DECLARE_METATYPE(Device*)

#endif // DEVICE_H
