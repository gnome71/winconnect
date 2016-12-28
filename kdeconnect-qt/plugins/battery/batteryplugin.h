#ifndef BATTERYPLUGIN_H
#define BATTERYPLUGIN_H

#include "batterypluginExport.h"
#include "batteryplugininterface.h"

//#include <QtCore>

#define PACKAGE_TYPE_BATTERY_REQUEST "kdeconnect.battery.request"

class Device;
class NetworkPackage;

class BATTERYPLUGIN_EXPORT BatteryPlugin
	: public BatteryPluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "at.winconnect.BatteryPlugin" FILE "batteryPlugin.json")
	Q_INTERFACES(BatteryPluginInterface)

public:
	QString info(const QString& name) Q_DECL_OVERRIDE;
	void initialize(const Device *device) Q_DECL_OVERRIDE;
	void connected() Q_DECL_OVERRIDE;
	int charge() const Q_DECL_OVERRIDE { return m_charge; }
	bool isCharging() const Q_DECL_OVERRIDE { return m_isCharging; }
	void updateValues(bool isCharging, int currentCharge) Q_DECL_OVERRIDE;

Q_SIGNALS:
	void stateChanged(bool charging);
	void chargeChanged(int charge);

private:
	Device *m_device;
	int m_charge = -1;
	bool m_isCharging = false;

	// Map to save current Interface for each device
	static QMap<QString, BatteryPlugin *> s_batteryInterfaces;
};

#endif // BATTERYPLUGIN_H
