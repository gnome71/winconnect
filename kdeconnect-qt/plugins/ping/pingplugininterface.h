#ifndef PINGPLUGININTERFACE_H
#define PINGPLUGININTERFACE_H

#pragma once

#include "pluginManagerExport.h"
#include "core/device.h"
#include "core/networkpackage.h"

#include <QObject>

/**
 * @brief The PingPluginInterface class
 *
 * Interface class for plugins
 */
class PLUGINMANAGER_EXPORT PingPluginInterface
		: public QObject
{
	Q_OBJECT

public:
	virtual ~PingPluginInterface(void) {}

	virtual void initialize(const Device *device, const QVariantList& args) = 0;
	virtual void connected() = 0;
	virtual void sendPing() const = 0;
	virtual void sendPing(const QString& customMessage) const = 0;
	virtual QString info(const QString& name) = 0;

public Q_SLOTS:
	virtual bool receivePackage(const NetworkPackage& np) = 0;
};

Q_DECLARE_INTERFACE(PingPluginInterface, "at.winconnect.PingPluginInterface")

#endif // PINGPLUGININTERFACE_H
