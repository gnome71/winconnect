#include "testplugina.h"
#include "core/device.h"

#include <QDebug>

QString TestPluginA::info(const QString & name)
{
	QString info = "TestPluginA info: " + name;
	return info;
}

