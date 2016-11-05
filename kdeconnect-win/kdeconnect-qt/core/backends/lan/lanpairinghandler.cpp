/**
 * Copyright 2015 Vineet Garg <grg.vineet@gmail.com>
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

#include <QDebug>

#include "core/daemon.h"
#include "core/kdeconnectconfig.h"
#include "core/kclogger.h"
#include "landevicelink.h"
#include "lanpairinghandler.h"
#include "core/networkpackagetypes.h"

LanPairingHandler::LanPairingHandler(DeviceLink* deviceLink)
    : PairingHandler(deviceLink)
    , m_status(NotPaired)
{
    m_pairingTimeout.setSingleShot(true);
    m_pairingTimeout.setInterval(30 * 1000);  //30 seconds of timeout
    connect(&m_pairingTimeout, &QTimer::timeout, this, &LanPairingHandler::pairingTimeout);
}

void LanPairingHandler::packageReceived(const NetworkPackage& np)
{
    m_pairingTimeout.stop();

    bool wantsPair = np.get<bool>("pair");

    if (wantsPair) {

        if (isPairRequested())  { //We started pairing

			qDebug() << "Pair answer";
			KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "Pair answer");
			setInternalPairStatus(Paired);
            
        } else {
			qDebug() << "Pair request";
			KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "Pair request");

            if (isPaired()) { //I'm already paired, but they think I'm not
                acceptPairing();
                return;
            }

            Daemon::instance()->askPairingConfirmation(this);
			//setInternalPairStatus(RequestedByPeer);
        }

    } else { //wantsPair == false

		qDebug() << "Unpair request";
		KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "UnPair request");

        setInternalPairStatus(NotPaired);
         if (isPairRequested()) {
			Q_EMIT pairingError("Canceled by other peer");
        }
    }
}

bool LanPairingHandler::requestPairing()
{
    switch (m_status) {
        case Paired:
			Q_EMIT pairingError(QString("%1: Already paired").arg(deviceLink()->name()));
            return false;
        case Requested:
			Q_EMIT pairingError(QString("%1: Pairing already requested for this device").arg(deviceLink()->name()));
            return false;
        case RequestedByPeer:
			qDebug() << deviceLink()->name() << " : Pairing already started by the other end, accepting their request.";
			KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, deviceLink()->name() + " : Pairing already started by the other end, accepting their request.");
			acceptPairing();
            return false;
        case NotPaired:
            ;
    }

    NetworkPackage np(PACKAGE_TYPE_PAIR, {{"pair", true}});
    const bool success = deviceLink()->sendPackage(np);
    if (success) {
        setInternalPairStatus(Requested);
        m_pairingTimeout.start();
    }
    return success;
}

bool LanPairingHandler::acceptPairing()
{
    m_pairingTimeout.stop(); // Just in case it is started
    NetworkPackage np(PACKAGE_TYPE_PAIR, {{"pair", true}});
    bool success = deviceLink()->sendPackage(np);
    if (success) {
        setInternalPairStatus(Paired);
		KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "Internal pairing status: paired");
	}
	else {
		KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "Internal pairing status: not paired");
	}
	return success;
}

void LanPairingHandler::rejectPairing()
{
    NetworkPackage np(PACKAGE_TYPE_PAIR, {{"pair", false}});
    deviceLink()->sendPackage(np);
    setInternalPairStatus(NotPaired);
}

void LanPairingHandler::unpair() {
    NetworkPackage np(PACKAGE_TYPE_PAIR, {{"pair", false}});
    deviceLink()->sendPackage(np);
    setInternalPairStatus(NotPaired);
}

void LanPairingHandler::pairingTimeout()
{
    NetworkPackage np(PACKAGE_TYPE_PAIR, {{"pair", false}});
    deviceLink()->sendPackage(np);
    setInternalPairStatus(NotPaired); //Will emit the change as well
	Q_EMIT pairingError("Timed out");
}

void LanPairingHandler::setInternalPairStatus(LanPairingHandler::InternalPairStatus status)
{
    m_status = status;
    if (status == Paired) {
		//KdeConnectConfig::instance()->setDeviceProperty(deviceLink()->deviceId(), "paired", "true");
        deviceLink()->setPairStatus(DeviceLink::Paired);
    } else {
		//KdeConnectConfig::instance()->setDeviceProperty(deviceLink()->deviceId(), "paired", "false");
		deviceLink()->setPairStatus(DeviceLink::NotPaired);
    }
}
