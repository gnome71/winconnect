#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#pragma once

#include "pluginManagerExport.h"

#include <QtCore/QObject>

class PluginManagerPrivate;

class PLUGINMANAGER_EXPORT PluginManager : public QObject
{
	Q_OBJECT

public:
	static PluginManager *instance(void);

	void initialize(void);
	void uninitialize(void);
	void scan(const QString& path);
	void load(const QString& path);
	void unload(const QString& path);
	QStringList plugins(void);

protected:
	 PluginManager(void);
	~PluginManager(void);

private:
	static PluginManager *s_instance;

private:
	PluginManagerPrivate *d;
};

#endif // PLUGINMANAGER_H
