#ifndef TESTPLUGINA_H
#define TESTPLUGINA_H

#include "testPluginAExport.h"
#include "plugins/plugininterface.h"

#include <QtCore>

class TESTPLUGINA_EXPORT TestPluginA
		: public PluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "at.winconnect.pluginInterface" FILE "testPluginA.json")
	Q_INTERFACES(PluginInterface)

public:
	QString info(const QString& name) Q_DECL_OVERRIDE;

	//explicit TestPluginA(QObject* parent = 0/*, const QVariantList& args*/);
	//~TestPluginA() override;
};

#endif // TESTPLUGINA_H
