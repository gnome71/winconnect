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

#include "device.h"

#include <QSslCertificate>
#include <QDebug>

#include "core/networkpackage.h"
#include "core/backends/linkprovider.h"

static void warn(const QString &info)
{
	qWarning() << "Device pairing error" << info;
}

Device::Device(QObject* parent, const QString& id)
	: QObject(parent)
	, m_deviceId(id)
	, m_protocolVersion(NetworkPackage::ProtocolVersion)
{
	KdeConnectConfig::DeviceInfo info = config.getTrustedDevice(id);

	m_deviceName = info.deviceName;
	m_deviceType = str2type(info.deviceType);

	connect(this, &Device::pairingError, this, &warn);
}

Device::Device(QObject* parent, const NetworkPackage& identityPackage, DeviceLink* dl)
	: QObject(parent)
	, m_deviceId(identityPackage.get<QString>("deviceId"))
	, m_deviceName(identityPackage.get<QString>("deviceName"))
{
	//addLink(identityPackage, dl);

	connect(this, &Device::pairingError, this, &warn);
}

bool Device::sendPackage(NetworkPackage &np)
{
	Q_ASSERT(np.type() != PACKAGE_TYPE_PAIR);
	Q_ASSERT(isTrusted());

	Q_FOREACH(DeviceLink* dl, m_deviceLinks) {
		if(dl->sendPackage(np)) return true;
	}
	return false;
}

Device::DeviceType Device::str2type(const QString &deviceType) {
	if (deviceType == "desktop") return Desktop;
	if (deviceType == "laptop") return Laptop;
	if (deviceType == "smartphone" || deviceType == "phone") return Phone;
	if (deviceType == "tablet") return Tablet;
	return Unknown;
}

QString Device::type2str(Device::DeviceType deviceType) {
	if (deviceType == Desktop) return "desktop";
	if (deviceType == Laptop) return "laptop";
	if (deviceType == Phone) return "smartphone";
	if (deviceType == Tablet) return "tablet";
	return "unknown";
}

void Device::setName(const QString &name)
{
	if (m_deviceName != name) {
		m_deviceName = name;
		Q_EMIT nameChanged(name);
	}
}

bool Device::isTrusted()
{
	return config.trustedDevices().contains(id());
}

QStringList Device::availableLinks()
{
	QStringList sl;
	Q_FOREACH(DeviceLink* dl, m_deviceLinks) {
		sl.append(dl->provider()->name());
	}
	return sl;
}

QString Device::encryptionInfo()
{
	QString result;
	QCryptographicHash::Algorithm digestAlgorithm = QCryptographicHash::Algorithm::Sha1;

	QString localSha1 = QString::fromLatin1(config.certificate().digest(digestAlgorithm).toHex());
	for (int i=2 ; i<localSha1.size() ; i+=3) {
		localSha1.insert(i, ':'); // Improve readability
	}
	result += tr("SHA1 fingerprint of your device certificate is: %1\n").arg(localSha1);

	std::string  remotePem = config.getDeviceProperty(id(), "certificate").toStdString();
	QSslCertificate remoteCertificate = QSslCertificate(QByteArray(remotePem.c_str(), remotePem.size()));
	QString remoteSha1 = QString::fromLatin1(remoteCertificate.digest(digestAlgorithm).toHex());
	for (int i=2 ; i<remoteSha1.size() ; i+=3) {
		remoteSha1.insert(i, ':'); // Improve readability
	}
	result += tr("SHA1 fingerprint of remote device certificate is: %1\n").arg(remoteSha1);

	return result;
}

Device::~Device()
{
//	qDeleteAll(m_deviceLinks);
//	m_deviceLinks.clear();
}

