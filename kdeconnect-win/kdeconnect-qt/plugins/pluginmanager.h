#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "pluginManagerExport.h"

#include <QtCore/QObject>

class PluginManagerPrivate;

class PLUGINMANAGER_EXPORT PluginManager : public QObject
{
	Q_OBJECT

public:
	static PluginManager *instance(void);

	void initialize();
	void uninitialize();
	void scan(const QString& path);
	void load(const QString& path);
	void unload(const QString& path);
	QStringList plugins(void);
	QString pluginName(const QString& path);
	QSet<QString> pluginsForCapabilities(const QSet<QString> &incoming, const QSet<QString> &outgoing);

protected:
	 PluginManager(void);
	~PluginManager(void);

private:
	static PluginManager *s_instance;
	PluginManagerPrivate *d;
};

#endif // PLUGINMANAGER_H
