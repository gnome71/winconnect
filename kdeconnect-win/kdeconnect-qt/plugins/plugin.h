#ifndef PLUGIN_H
#define PLUGIN_H

#pragma once

#include "pluginManagerExport.h"
//#include "core/device.h"

#include <QObject>

/**
 * @brief The Plugin class
 *
 * Interface class for plugins
 */
class PLUGINMANAGER_EXPORT Plugin : public QObject
{
	Q_OBJECT

public:
	Plugin() {}
	virtual ~Plugin(void) {}

};

Q_DECLARE_INTERFACE(Plugin, "at.winconnect.plugin")

#endif // PLUGIN_H
