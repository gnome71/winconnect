/**
 * Copyright 2014 Albert Vaca <albertvaka@gmail.com>
 * Modified for Windows by Alexander Kaspar <alexander.kaspar@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PINGPLUGIN_H
#define PINGPLUGIN_H

#include "pingpluginExport.h"
#include "pingplugininterface.h"

#define PACKAGE_TYPE_PING "kdeconnect.ping"

class Device;
class NetworkPackage;

class PINGPLUGIN_EXPORT PingPlugin
    : public PingPluginInterface	//KdeConnectPlugin
{
    Q_OBJECT
	Q_PLUGIN_METADATA(IID "at.winconnect.PingPlugin" FILE "pingPlugin.json")
	Q_INTERFACES(PingPluginInterface)

public:
	void initialize(const Device *device, const QVariantList& args) Q_DECL_OVERRIDE;	
	void connected() Q_DECL_OVERRIDE;
	void sendPing() const Q_DECL_OVERRIDE;
    void sendPing(const QString& customMessage) const Q_DECL_OVERRIDE;
	QString info(const QString& name) Q_DECL_OVERRIDE;

public Q_SLOTS:
    bool receivePackage(const NetworkPackage& np) override;
    //void connected() override;

private:
	Device *m_device;
};

#endif	// PINGPLUGIN_H
