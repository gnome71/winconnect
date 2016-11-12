#include "testplugina.h"
#include "core/device.h"

#include <QDebug>

void TestPluginA::sendPing(const QVariantList &args)
{
	qDebug() << "TestPluginA: sendPing ->" << args.at(0).toString();
}

QString TestPluginA::info(const QString & name)
{
	QString info = "TestPluginA info: " + name;
	return info;
}

