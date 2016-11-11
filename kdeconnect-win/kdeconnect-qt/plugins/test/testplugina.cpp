#include "testplugina.h"
#include "core/device.h"

Q_PLUGIN_METADATA(IID "at.winconnect.plugin")

TestPluginA::TestPluginA()
	: Plugin()
{
	qDebug() << "Testplugin constructor";
}

TestPluginA::~TestPluginA()
{
	qDebug() << "Testplugin destructor";
}

//#include "testplugina.moc"

