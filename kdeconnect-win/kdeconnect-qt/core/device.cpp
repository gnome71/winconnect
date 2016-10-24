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

#ifdef interface // MSVC language extension, QDBusConnection uses this as a variable name
#undef interface
#endif

//#include <QDBusConnection>
#include <QSslCertificate>

//#include <KSharedConfig>
//#include <KConfigGroup>
//#include <KLocalizedString>

#include "kclogger.h"
//#include "kdeconnectplugin.h"
//#include "pluginloader.h"
#include "backends/devicelink.h"
#include "backends/linkprovider.h"
#include "networkpackage.h"
#include "kdeconnectconfig.h"
#include "daemon.h"

static void warn(const QString &info)
{
	qWarning() << "Device pairing error" << info;
}

/**
 * @brief Device::Device known, trusted but waiting for link
 * @param parent
 * @param id
 */
Device::Device(QObject* parent, const QString& id)
	: QObject(parent)
	, m_deviceId(id)
	, m_protocolVersion(NetworkPackage::ProtocolVersion) //We don't know it yet
{
	KdeConnectConfig* config = new KdeConnectConfig();
	KdeConnectConfig::DeviceInfo info = config->getTrustedDevice(id);

	m_deviceName = info.deviceName;
	m_deviceType = str2type(info.deviceType);

	//Register in bus
	//QDBusConnection::sessionBus().registerObject(dbusPath(), this, QDBusConnection::ExportScriptableContents | QDBusConnection::ExportAdaptors);

	//Assume every plugin is supported until addLink is called and we can get the actual list
	//m_supportedPlugins = PluginLoader::instance()->getPluginList().toSet();

	connect(this, &Device::pairingError, this, &warn);
}

/**
 * @brief Device::Device known but not trusted
 * @param parent
 * @param identityPackage
 * @param dl
 */
Device::Device(QObject* parent, const NetworkPackage& identityPackage, DeviceLink* dl)
	: QObject(parent)
	, m_deviceId(identityPackage.get<QString>("deviceId"))
	, m_deviceName(identityPackage.get<QString>("deviceName"))
{
	addLink(identityPackage, dl);

	//Register in bus
	//QDBusConnection::sessionBus().registerObject(dbusPath(), this, QDBusConnection::ExportScriptableContents | QDBusConnection::ExportAdaptors);

	connect(this, &Device::pairingError, this, &warn);
}

Device::~Device()
{
	qDeleteAll(m_deviceLinks);
	m_deviceLinks.clear();
}

void Device::requestPair()
{
	if (isTrusted()) {
		Q_EMIT pairingError("Already paired");
		return;
	}

	if (!isReachable()) {
		Q_EMIT pairingError("Device not reachable");
		return;
	}

	Q_FOREACH(DeviceLink* dl, m_deviceLinks) {
		dl->userRequestsPair();
	}
}

void Device::unpair()
{
	KdeConnectConfig* config = new KdeConnectConfig();
	Q_FOREACH(DeviceLink* dl, m_deviceLinks) {
		dl->userRequestsUnpair();
	}
	config->removeTrustedDevice(id());
}

void Device::pairStatusChanged(DeviceLink::PairStatus status)
{
	KdeConnectConfig* config = new KdeConnectConfig();
	if (status == DeviceLink::NotPaired) {
		config->removeTrustedDevice(id());

		Q_FOREACH(DeviceLink* dl, m_deviceLinks) {
			if (dl != sender()) {
				dl->setPairStatus(DeviceLink::NotPaired);
			}
		}
	} else {
		config->addTrustedDevice(id(), name(), type());
	}

	//reloadPlugins(); //Will load/unload plugins

	bool isTrusted = (status == DeviceLink::Paired);
	Q_EMIT trustedChanged(isTrusted? Trusted : NotTrusted);
	Q_ASSERT(isTrusted == this->isTrusted());
}

static bool lessThan(DeviceLink* p1, DeviceLink* p2)
{
	return p1->provider()->priority() > p2->provider()->priority();
}

void Device::addLink(const NetworkPackage& identityPackage, DeviceLink* link)
{
	qDebug() << "Adding link to" << id() << "via" << link->provider();

	Q_ASSERT(!m_deviceLinks.contains(link));

	m_protocolVersion = identityPackage.get<int>("protocolVersion", -1);
	if (m_protocolVersion != NetworkPackage::ProtocolVersion) {
		qWarning() << m_deviceName << "- warning, device uses a different protocol version" << m_protocolVersion << "expected" << NetworkPackage::ProtocolVersion;
	}

	connect(link, SIGNAL(destroyed(QObject*)),
			this, SLOT(linkDestroyed(QObject*)));

	m_deviceLinks.append(link);

	//re-read the device name from the identityPackage because it could have changed
	setName(identityPackage.get<QString>("deviceName"));
	m_deviceType = str2type(identityPackage.get<QString>("deviceType"));

	//Theoretically we will never add two links from the same provider (the provider should destroy
	//the old one before this is called), so we do not have to worry about destroying old links.
	//-- Actually, we should not destroy them or the provider will store an invalid ref!

	connect(link, SIGNAL(receivedPackage(NetworkPackage)),
			this, SLOT(privateReceivedPackage(NetworkPackage)));

	qSort(m_deviceLinks.begin(), m_deviceLinks.end(), lessThan);

	const bool capabilitiesSupported = identityPackage.has("incomingCapabilities") || identityPackage.has("outgoingCapabilities");
	if (capabilitiesSupported) {
		const QSet<QString> outgoingCapabilities = identityPackage.get<QStringList>("outgoingCapabilities").toSet()
						  , incomingCapabilities = identityPackage.get<QStringList>("incomingCapabilities").toSet();

//		m_supportedPlugins = PluginLoader::instance()->pluginsForCapabilities(incomingCapabilities, outgoingCapabilities);
		//qDebug() << "new plugins for" << m_deviceName << m_supportedPlugins << incomingCapabilities << outgoingCapabilities;
	} else {
//		m_supportedPlugins = PluginLoader::instance()->getPluginList().toSet();
	}

	//reloadPlugins();

	if (m_deviceLinks.size() == 1) {
		Q_EMIT reachableStatusChanged();
	}

	connect(link, &DeviceLink::pairStatusChanged, this, &Device::pairStatusChanged);
	connect(link, &DeviceLink::pairingError, this, &Device::pairingError);
}

void Device::linkDestroyed(QObject* o)
{
	removeLink(static_cast<DeviceLink*>(o));
}

void Device::removeLink(DeviceLink* link)
{
	m_deviceLinks.removeAll(link);

	//qCDebug(KDECONNECT_CORE) << "RemoveLink" << m_deviceLinks.size() << "links remaining";

	if (m_deviceLinks.isEmpty()) {
		//reloadPlugins();
		Q_EMIT reachableStatusChanged();
	}
}

bool Device::sendPackage(NetworkPackage& np)
{
	Q_ASSERT(np.type() != PACKAGE_TYPE_PAIR);
	Q_ASSERT(isTrusted());

	//Maybe we could block here any package that is not an identity or a pairing package to prevent sending non encrypted data
	Q_FOREACH(DeviceLink* dl, m_deviceLinks) {
		if (dl->sendPackage(np)) return true;
	}

	return false;
}

void Device::privateReceivedPackage(const NetworkPackage& np)
{
	Q_ASSERT(np.type() != PACKAGE_TYPE_PAIR);
	if (isTrusted()) {
//		const QList<KdeConnectPlugin*> plugins = m_pluginsByIncomingCapability.values(np.type());
//		if (plugins.isEmpty()) {
//			qWarning() << "discarding unsupported package" << np.type() << "for" << name();
//		}
//		Q_FOREACH (KdeConnectPlugin* plugin, plugins) {
//			plugin->receivePackage(np);
//		}
	} else {
		qDebug() << "device" << name() << "not paired, ignoring package" << np.type();
		unpair();
	}

}

bool Device::isTrusted() const
{
	KdeConnectConfig* config = new KdeConnectConfig();
	return config->trustedDevices().contains(id());
}

QStringList Device::availableLinks() const
{
	QStringList sl;
	Q_FOREACH(DeviceLink* dl, m_deviceLinks) {
		sl.append(dl->provider()->name());
	}
	return sl;
}

void Device::cleanUnneededLinks() {
	if (isTrusted()) {
		return;
	}
	for(int i = 0; i < m_deviceLinks.size(); ) {
		DeviceLink* dl = m_deviceLinks[i];
		if (!dl->linkShouldBeKeptAlive()) {
			dl->deleteLater();
			m_deviceLinks.remove(i);
		} else {
			i++;
		}
	}
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

QString Device::statusIconName() const
{
	return iconForStatus(isReachable(), isTrusted());
}

QString Device::iconName() const
{
	return iconForStatus(true, false);
}

QString Device::iconForStatus(bool reachable, bool trusted) const
{
	Device::DeviceType deviceType = m_deviceType;
	if (deviceType == Device::Unknown) {
		deviceType = Device::Phone; //Assume phone if we don't know the type
	} else if (deviceType == Device::Desktop) {
		deviceType = Device::Device::Laptop; // We don't have desktop icon yet
	}

	QString status = (reachable? (trusted? QStringLiteral("connected") : QStringLiteral("disconnected")) : QStringLiteral("trusted"));
	QString type = type2str(deviceType);

	return type+'-'+status;
}

void Device::setName(const QString &name)
{
	if (m_deviceName != name) {
		m_deviceName = name;
		Q_EMIT nameChanged(name);
	}
}

QString Device::encryptionInfo() const
{
	KdeConnectConfig* config = new KdeConnectConfig();
	QString result;
	QCryptographicHash::Algorithm digestAlgorithm = QCryptographicHash::Algorithm::Sha1;

	QString localSha1 = QString::fromLatin1(config->certificate().digest(digestAlgorithm).toHex());
	for (int i=2 ; i<localSha1.size() ; i+=3) {
		localSha1.insert(i, ':'); // Improve readability
	}
	result += "SHA1 fingerprint of your device certificate is: " + localSha1 + "\n";

	std::string  remotePem = config->getDeviceProperty(id(), "certificate").toStdString();
	QSslCertificate remoteCertificate = QSslCertificate(QByteArray(remotePem.c_str(), remotePem.size()));
	QString remoteSha1 = QString::fromLatin1(remoteCertificate.digest(digestAlgorithm).toHex());
	for (int i=2 ; i<remoteSha1.size() ; i+=3) {
		remoteSha1.insert(i, ':'); // Improve readability
	}
	result += "SHA1 fingerprint of remote device certificate is: " + remoteSha1 + "\n";

	qDebug() << "EncryptionInfo:" << result;

	return result;
}

