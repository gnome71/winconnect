#ifndef TESTPLUGINA_H
#define TESTPLUGINA_H

#pragma once

#include "testPluginAExport.h"
#include "plugins/plugin.h"

#include <QtCore>

class TESTPLUGINA_EXPORT TestPluginA
		: public Plugin
{
	Q_INTERFACES(Plugin)
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "at.winconnect.plugin" FILE "testPluginA.json")

public:
	TestPluginA();
	~TestPluginA() override;

signals:

public slots:
};

#endif // TESTPLUGINA_H
