#include "batteryplugin.h"

#include <QDebug>

void BatteryPlugin::initialize(const Device *device)
{
	m_device = const_cast<Device*>(device);
}

void BatteryPlugin::connected()
{
	//TODO:NetworkPackage np(PACKAGE_TYPE_BATTERY_REQUEST, { {"request", true} });
	//TODO:m_device->sendPackage(np);
}

void BatteryPlugin::updateValues(bool isCharging, int currentCharge)
{

}

QString BatteryPlugin::info(const QString & name)
{
	QString info = "BatteryPlugin for: " + m_device->name();
	return info;
}
