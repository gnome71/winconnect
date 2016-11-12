#ifndef TESTPLUGINA_H
#define TESTPLUGINA_H

#include "testPluginAExport.h"
#include "plugins/testplugininterface.h"

#include <QtCore>

class TESTPLUGINA_EXPORT TestPluginA
		: public TestPluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "at.winconnect.TestPluginInterface" FILE "testPluginA.json")
	Q_INTERFACES(TestPluginInterface)

public:
	//TestPluginA(QObject* parent = 0);
	//~TestPluginA() override;
	QString info(const QString& name) Q_DECL_OVERRIDE;
	void sendPing(const QVariantList &args) Q_DECL_OVERRIDE;
};

#endif // TESTPLUGINA_H
