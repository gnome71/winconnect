#ifndef TESTPLUGINAINTERFACE_H
#define TESTPLUGINAINTERFACE_H

#pragma once

#include "pluginManagerExport.h"
#include <QObject>

/**
 * @brief The TestPluginAInterface class
 *
 * Interface class for plugins
 */
class PLUGINMANAGER_EXPORT TestPluginAInterface
		: public QObject
{
	Q_OBJECT

public:
	virtual ~TestPluginAInterface(void) {}

	virtual QString info(const QString& name) = 0;

Q_SIGNALS:
	//virtual void stateChanged(bool charging) = 0;
	//virtual void chargeChanged(int charge) = 0;
};

Q_DECLARE_INTERFACE(TestPluginAInterface, "at.winconnect.TestPluginAInterface")

#endif // TESTPLUGINAINTERFACE_H
