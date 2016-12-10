#include "batteryplugin.h"

#include <QDebug>

void BatteryPlugin::updateValues(bool isCharging, int currentCharge)
{

}

QString BatteryPlugin::info(const QString & name)
{
	QString info = "BatteryPlugin info: " + name;
	return info;
}
