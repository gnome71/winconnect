#include "plugininterface.h"
#include "battery/batteryplugininterface.h"
#include "pluginmanager.h"

#include <QtCore>
#include <QtDebug>

/**
 * @brief The PluginManagerPrivate class
 */
class PluginManagerPrivate
{
public:
	bool check(const QString& path);

public:
	QHash<QString, QVariant> names;
	QHash<QString, QVariant> versions;
	QHash<QString, QVariantList> dependencies;
	QHash<QString, QVariantList> outgoing;
	QHash<QString, QVariantList> supported;

public:
	QHash<QString, QPluginLoader *> loaders;
};

bool PluginManagerPrivate::check(const QString& path)
{
	bool status = true;

	foreach(QVariant item, this->dependencies.value(path)) {

		QVariantMap mitem = item.toMap();
		QVariant na_mitem = mitem.value("name");
		QVariant ve_mitem = mitem.value("version");
		QString key = this->names.key(na_mitem);

		if(!this->names.values().contains(na_mitem)) {
			qDebug() << Q_FUNC_INFO << "  Missing dependency:" << na_mitem.toString() << "for plugin" << path;
			status = false;
			continue;
		}

		if (this->versions.value(key) != ve_mitem) {
			qDebug() << Q_FUNC_INFO << "    Version mismatch:" << na_mitem.toString() << "version" << this->versions.value(this->names.key(na_mitem)).toString() << "but" << ve_mitem.toString() << "required for plugin" << path;
			status = false;
			continue;
		}

		if(!check(key)) {
			qDebug() << Q_FUNC_INFO << "Corrupted dependency:" << na_mitem.toString() << "for plugin" << path;
			status = false;
			continue;
		}
	}

	return status;
}

/**
 * @brief PluginManager::instance
 * @return
 */
PluginManager *PluginManager::instance(void)
{
	if(!s_instance)
		s_instance = new PluginManager;

	return s_instance;
}

void PluginManager::initialize(void)
{
	qDebug() << "PluginManager: initialize";

	QDir path = qApp->applicationDirPath();
	path.cd("plugins");
	foreach(QFileInfo info, path.entryInfoList(QDir::Files | QDir::NoDotAndDotDot))
		this->scan(info.absoluteFilePath());

	foreach(QFileInfo info, path.entryInfoList(QDir::Files | QDir::NoDotAndDotDot))
		this->load(info.absoluteFilePath());
}

void PluginManager::uninitialize(void)
{
	foreach(const QString &path, d->loaders.keys())
		this->unload(path);
}

void PluginManager::scan(const QString& path)
{
	//qDebug() << "PluginManager: scan" << path;
	if(!QLibrary::isLibrary(path))
		return;

	QPluginLoader *loader = new QPluginLoader(path);

	d->names.insert(path, loader->metaData().value("MetaData").toObject().value("name").toVariant());
	d->versions.insert(path, loader->metaData().value("MetaData").toObject().value("version").toVariant());
	d->dependencies.insert(path, loader->metaData().value("MetaData").toObject().value("dependencies").toArray().toVariantList());
	d->outgoing.insert(path, loader->metaData().value("MetaData").toObject().value("X-WinConnect-OutgoingPackageType").toArray().toVariantList());
	d->supported.insert(path, loader->metaData().value("MetaData").toObject().value("X-WinConnect-SupportedPackageType").toArray().toVariantList());
	
	delete loader;
}

void PluginManager::load(const QString& path)
{
	if(!QLibrary::isLibrary(path))
		return;

	if(!d->check(path))
		return;

	QPluginLoader *loader = new QPluginLoader(path);

	if(PluginInterface *plugin = qobject_cast<PluginInterface *>(loader->instance()))
		d->loaders.insert(path, loader);
	else if(BatteryPluginInterface *plugin = qobject_cast<BatteryPluginInterface *>(loader->instance()))
		d->loaders.insert(path, loader);
	else
		delete loader;
}

void PluginManager::unload(const QString& path)
{
	QPluginLoader *loader = d->loaders.value(path);

	if(loader->unload()) {
		d->loaders.remove(path);
		delete loader;
	}
}

QStringList PluginManager::plugins(void)
{
	qDebug() << "Plugins:" << d->loaders;
	return d->loaders.keys();
}

QString PluginManager::pluginName(const QString& path)
{
	//qDebug() << "Loaders:" << d->loaders;
	return d->names.value(path).toString();
}

QSet<QString> PluginManager::pluginsForCapabilities(const QSet<QString> &incoming, const QSet<QString> &outgoing)
{
	QSet<QString> ret;
	
	// TODO: for loop
	for (const QString &service : d->loaders.keys()) {
		qDebug() << "service key: "  << service;
		const QSet<QVariantList> pluginIncomingCapabilities = d->supported.values().toSet();
		const QSet<QVariantList> pluginOutgoingCapabilities = d->outgoing.values().toSet();
		qDebug() << "Incoming cap: " << pluginIncomingCapabilities;
		qDebug() << "Outgoing cap: " << pluginOutgoingCapabilities;
	}

	return ret;
}

PluginManager::PluginManager(void) : d(new PluginManagerPrivate)
{

}

PluginManager::~PluginManager(void)
{
	delete d;

	d = NULL;
}

PluginManager *PluginManager::s_instance = NULL;
