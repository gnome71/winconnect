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

#include "kclogger.h"
#include "kdeconnectplugin.h"
#include "plugins/pluginmanager.h"
#include "backends/devicelink.h"
#include "backends/linkprovider.h"
#include "networkpackage.h"
#include "kdeconnectconfig.h"
#include "daemon.h"

/**
 * @brief warn
 * @param info
 */
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
	KdeConnectConfig::DeviceInfo info = KdeConnectConfig::instance()->getTrustedDevice(id);

	m_deviceName = info.deviceName;
	m_deviceType = str2type(info.deviceType);
	// TODO: NotificationInterface* m_notificationInterface = new NotificationInterface(this, m_deviceId);
	//Assume every plugin is supported until addLink is called and we can get the actual list
	PluginManager::instance()->initialize();
	m_supportedPlugins = PluginManager::instance()->plugins().toSet();
	qDebug() << "## m_supportedPlugins: " << m_supportedPlugins;

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

	// TODO: NotificationInterface* m_notificationInterface = new NotificationInterface(this, m_deviceId);

	connect(this, &Device::pairingError, this, &warn);
}

/**
 * @brief Device::~Device
 */
Device::~Device()
{
	qDeleteAll(m_deviceLinks);
	m_deviceLinks.clear();
	//delete m_notifications;
}

QString Device::pluginsConfigFile() const
{
	qDebug() << "Device config dir:" << KdeConnectConfig::instance()->deviceConfigDir(id()).absoluteFilePath("config");
	return KdeConnectConfig::instance()->deviceConfigDir(id()).absoluteFilePath("config");
}

bool Device::hasPlugin(const QString& name) const
{
	return m_plugins.contains(name);
}

void Device::setPluginEnabled(const QString& pluginName, bool enabled)
{
	//KConfigGroup pluginStates = KSharedConfig::openConfig(pluginsConfigFile())->group("Plugins");
	m_plugin_config = new KdeConnectPluginConfig(m_deviceId, pluginName);

	const QString enabledKey = pluginName + QStringLiteral("Enabled");
	m_plugin_config->set(m_deviceId, enabledKey);
	//pluginStates.writeEntry(enabledKey, enabled);
	m_plugin_config->deleteLater();
	reloadPlugins();
}

bool Device::isPluginEnabled(const QString& pluginName) const
{
	const QString enabledKey = pluginName + QStringLiteral("Enabled");
	//KConfigGroup pluginStates = KSharedConfig::openConfig(pluginsConfigFile())->group("Plugins");

	//return (pluginStates.hasKey(enabledKey) ? pluginStates.readEntry(enabledKey, false)
	//	: PluginLoader::instance()->getPluginInfo(pluginName).isEnabledByDefault());
	return true;	// TODO: isPluginEnabled
}

void Device::reloadPlugins()
{
	QHash<QString, KdeConnectPlugin*> newPluginMap, oldPluginMap = m_plugins; //TODO: m_plugins=0
	QMultiMap<QString, KdeConnectPlugin*> newPluginsByIncomingCapability;

	if (isTrusted() && isReachable()) {
		PluginManager* loader = PluginManager::instance();

		for (const QString& pluginName : m_supportedPlugins) {
			qDebug() << prefix << "m_supportedPlugin:" << pluginName;
			const bool pluginEnabled = isPluginEnabled(pluginName);
			const QSet<QString> incomingCapabilities = loader->getInPluginInfo(pluginName);
		
			if (pluginEnabled) {
				loader->load(pluginName);
				qDebug() << "Device:m_plugins" << m_plugins;
				KdeConnectPlugin* plugin = m_plugins.take(pluginName);
			}
		}
	}
	//pluginsConfigFile();	//DEBUG
}

/**
 * @brief Device::requestPair
 */
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

/**
 * @brief Device::unpair
 */
void Device::unpair()
{
	Q_FOREACH(DeviceLink* dl, m_deviceLinks) {
		dl->userRequestsUnpair();
	}
	KdeConnectConfig::instance()->removeTrustedDevice(id());
}

/**
 * @brief Device::pairStatusChanged
 * @param status
 */
void Device::pairStatusChanged(DeviceLink::PairStatus status)
{
	if (status == DeviceLink::NotPaired) {
		KdeConnectConfig::instance()->removeTrustedDevice(id());

		Q_FOREACH(DeviceLink* dl, m_deviceLinks) {
			if (dl != sender()) {
				dl->setPairStatus(DeviceLink::NotPaired);
			}
		}
	} else {
		KdeConnectConfig::instance()->addTrustedDevice(id(), name(), type());
	}

	reloadPlugins(); //Will load/unload plugins

	bool isTrusted = (status == DeviceLink::Paired);
	Q_EMIT trustedChanged(isTrusted? Trusted : NotTrusted);
	Q_ASSERT(isTrusted == this->isTrusted());
}

/**
 * @brief lessThan
 * @param p1
 * @param p2
 * @return
 */
static bool lessThan(DeviceLink* p1, DeviceLink* p2)
{
	return p1->provider()->priority() > p2->provider()->priority();
}

/**
 * @brief Device::addLink
 * @param identityPackage
 * @param link
 */
void Device::addLink(const NetworkPackage& identityPackage, DeviceLink* link)
{
	qDebug() << "Device: adding link to" << id() << "via" << link->provider();
	KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "Adding link to " + id() + " via " + link->provider()->name());

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

		m_supportedPlugins = PluginManager::instance()->pluginsForCapabilities(incomingCapabilities, outgoingCapabilities);
		
		KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "New plugins for: " + m_deviceName);
		for (const QString &s : qAsConst(m_supportedPlugins)) {
			KcLogger::instance()->write(QtMsgType::QtDebugMsg, prefix, "  " 
				+ PluginManager::instance()->pluginName(s));
		}

		qDebug() << "New plugins for" << m_deviceName << m_supportedPlugins;
	} else {
		m_supportedPlugins = PluginManager::instance()->plugins().toSet();
	}

	for (const QString & n : m_supportedPlugins)
		setPluginEnabled(n, true);

	reloadPlugins();

	if (m_deviceLinks.size() == 1) {
		Q_EMIT reachableStatusChanged();
	}

	connect(link, &DeviceLink::pairStatusChanged, this, &Device::pairStatusChanged);
	connect(link, &DeviceLink::pairingError, this, &Device::pairingError);
}

/**
 * @brief Device::linkDestroyed
 * @param o
 */
void Device::linkDestroyed(QObject* o)
{
	removeLink(static_cast<DeviceLink*>(o));
}

/**
 * @brief Device::removeLink
 * @param link
 */
void Device::removeLink(DeviceLink* link)
{
	m_deviceLinks.removeAll(link);

	//qCDebug(KDECONNECT_CORE) << "RemoveLink" << m_deviceLinks.size() << "links remaining";

	if (m_deviceLinks.isEmpty()) {
		//reloadPlugins();
		Q_EMIT reachableStatusChanged();
	}
}

/**
 * @brief Device::sendPackage
 * @param np
 * @return
 */
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

/**
 * @brief Device::privateReceivedPackage
 * @param np
 */
void Device::privateReceivedPackage(const NetworkPackage& np)
{
	Q_ASSERT(np.type() != PACKAGE_TYPE_PAIR);
	if (isTrusted()) {
		const QList<KdeConnectPlugin*> plugins = m_pluginsByIncomingCapability.values(np.type());
		if (plugins.isEmpty()) {
			qWarning() << prefix << "discarding unsupported package" << np.type() << "for" << name();
		}
		Q_FOREACH (KdeConnectPlugin* plugin, plugins) {
			plugin->receivePackage(np);
		}
	} else {
		qDebug() << prefix << "Device: device" << name() << "not paired, ignoring package" << np.type();
		unpair();
	}

}

/**
 * @brief Device::isTrusted
 * @return
 */
bool Device::isTrusted() const
{
	return KdeConnectConfig::instance()->trustedDevices().contains(id());
}

Q_SCRIPTABLE QStringList Device::loadedPlugins() const
{
	return Q_SCRIPTABLE QStringList();
}

/**
 * @brief Device::availableLinks
 * @return
 */
QStringList Device::availableLinks() const
{
	QStringList sl;
	Q_FOREACH(DeviceLink* dl, m_deviceLinks) {
		sl.append(dl->provider()->name());
	}
	return sl;
}

/**
 * @brief Device::cleanUnneededLinks
 */
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

/**
 * @brief Device::str2type
 * @param deviceType
 * @return Device type
 */
Device::DeviceType Device::str2type(const QString &deviceType) {
	if (deviceType == "desktop") return Desktop;
	if (deviceType == "laptop") return Laptop;
	if (deviceType == "smartphone" || deviceType == "phone") return Phone;
	if (deviceType == "tablet") return Tablet;
	return Unknown;
}

/**
 * @brief Device::type2str
 * @param deviceType
 * @return deviceType
 */
QString Device::type2str(Device::DeviceType deviceType) {
	if (deviceType == Desktop) return "desktop";
	if (deviceType == Laptop) return "laptop";
	if (deviceType == Phone) return "smartphone";
	if (deviceType == Tablet) return "tablet";
	return "unknown";
}

/**
 * @brief Device::statusIconName
 * @return IconName
 */
QString Device::statusIconName() const
{
	return iconForStatus(isReachable(), isTrusted());
}

QString Device::iconName() const
{
	return iconForStatus(true, false);
}

/**
 * @brief Device::iconForStatus
 * @param reachable
 * @param trusted
 * @return IconName
 */
QString Device::iconForStatus(bool reachable, bool trusted) const
{
	Device::DeviceType deviceType = m_deviceType;
	if (deviceType == Device::Unknown) {
		deviceType = Device::Phone; //Assume phone if we don't know the type
	} else if (deviceType == Device::Desktop) {
		deviceType = Device::Device::Laptop; // We don't have desktop icon yet
	}

	QString status;
	if(reachable && trusted)
		status = "trusted";
	else if (reachable && !trusted)
		status = "connected";
	else
		status = "disconnected";

	QString type = type2str(deviceType);

	return type+'-'+status;
}

/**
 * @brief Device::setName
 * @param name
 */
void Device::setName(const QString &name)
{
	if (m_deviceName != name) {
		m_deviceName = name;
		Q_EMIT nameChanged(name);
	}
}

/**
 * @brief Device::encryptionInfo
 * @return Encryption info for device
 */
QString Device::encryptionInfo() const
{
	QString result;
	QCryptographicHash::Algorithm digestAlgorithm = QCryptographicHash::Algorithm::Sha1;

	QString localSha1 = QString::fromLatin1(KdeConnectConfig::instance()->certificate().digest(digestAlgorithm).toHex());
	for (int i=2 ; i<localSha1.size() ; i+=3) {
		localSha1.insert(i, ':'); // Improve readability
	}
	result += "Local SHA1 fingerprint: " + localSha1;

	std::string  remotePem = KdeConnectConfig::instance()->getDeviceProperty(m_deviceId, "certificate").toStdString();
	QSslCertificate remoteCertificate = QSslCertificate(QByteArray(remotePem.c_str(), remotePem.size()));
	QString remoteSha1 = QString::fromLatin1(remoteCertificate.digest(digestAlgorithm).toHex());
	for (int i=2 ; i<remoteSha1.size() ; i+=3) {
		remoteSha1.insert(i, ':'); // Improve readability
	}
	result += ", remote SHA1 fingerprint: " + remoteSha1;

	qDebug() << "Device: EncryptionInfo:" << result;
	Q_EMIT const_cast<Device*>(this)->logMe(QtMsgType::QtInfoMsg, prefix, "Encryption Info: " + result);

	return result;
}

