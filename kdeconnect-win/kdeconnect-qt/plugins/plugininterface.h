#ifndef PLUGIN_H
#define PLUGIN_H

#pragma once

#include "pluginManagerExport.h"
//#include "core/device.h"

#include <QObject>

/**
 * @brief The PluginInterface class
 *
 * Interface class for plugins
 */
class PLUGINMANAGER_EXPORT PluginInterface
		: public QObject
{
	Q_OBJECT

public:
	virtual ~PluginInterface(void) {}

	virtual QString info(const QString& name) = 0;
};

Q_DECLARE_INTERFACE(PluginInterface, "at.winconnect.pluginInterface")

#endif // PLUGIN_H
