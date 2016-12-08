#include "testplugina.h"
#include "core/device.h"

#include <QDebug>

void TestPluginA::sendPing(const QVariantList &args)
{
	for (QVariantList::const_iterator i = args.begin(); i != args.end(); i++)
	{
		qDebug() << "TestPluginA: arg" << (*i).toString();
	}
	qDebug() << "TestPluginA: sendPing ->" << args.at(0).toString();
}

QString TestPluginA::info(const QString & name)
{
	QString info = "TestPluginA info: " + name;
	return info;
}

