#ifndef TESTPLUGINA_H
#define TESTPLUGINA_H

#include "testPluginAExport.h"
#include "plugins/plugininterface.h"

#include <QtCore>

class TESTPLUGINA_EXPORT TestPluginA
		: public PluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "at.winconnect.PluginInterface" FILE "testPluginA.json")
	Q_INTERFACES(PluginInterface)

public:
	//TestPluginA(QObject* parent = 0);
	//~TestPluginA() override;
	QString info(const QString& name) Q_DECL_OVERRIDE;
	void sendPing(const QVariantList &args) Q_DECL_OVERRIDE;
};

#endif // TESTPLUGINA_H
