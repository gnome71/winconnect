#ifndef BATTERYPLUGININTERFACE_H
#define BATTERYPLUGININTERFACE_H

#pragma once

#include "pluginManagerExport.h"
#include <QObject>

/**
 * @brief The BatteryPluginInterface class
 *
 * Interface class for plugins
 */
class PLUGINMANAGER_EXPORT BatteryPluginInterface
		: public QObject
{
	Q_OBJECT

public:
	
	virtual ~BatteryPluginInterface(void) {}

	virtual int charge() const = 0;
	virtual bool isCharging() const = 0;
	virtual void updateValues(bool isCharging, int currentCharge) = 0;
	virtual QString info(const QString& name) = 0;

Q_SIGNALS:
	//virtual void stateChanged(bool charging) = 0;
	//virtual void chargeChanged(int charge) = 0;
};

Q_DECLARE_INTERFACE(BatteryPluginInterface, "at.winconnect.BatteryPluginInterface")

#endif // BATTERYPLUGININTERFACE_H
