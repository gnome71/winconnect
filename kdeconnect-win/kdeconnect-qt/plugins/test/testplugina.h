#ifndef TESTPLUGINA_H
#define TESTPLUGINA_H

#include "testPluginAExport.h"
#include "testpluginainterface.h"

#include <QtCore>

class TESTPLUGINA_EXPORT TestPluginA
		: public TestPluginAInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "at.winconnect.TestPluginA" FILE "testPluginA.json")
	Q_INTERFACES(TestPluginAInterface)

public:
	QString info(const QString& name) Q_DECL_OVERRIDE;
};

#endif // TESTPLUGINA_H
