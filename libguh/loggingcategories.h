/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef LOGGINGCATEGORYS_H
#define LOGGINGCATEGORYS_H

#include <QLoggingCategory>

// Core / libguh
Q_DECLARE_LOGGING_CATEGORY(dcApplication)
Q_DECLARE_LOGGING_CATEGORY(dcDeviceManager)
Q_DECLARE_LOGGING_CATEGORY(dcRuleEngine)
Q_DECLARE_LOGGING_CATEGORY(dcHardware)
Q_DECLARE_LOGGING_CATEGORY(dcConnection)
Q_DECLARE_LOGGING_CATEGORY(dcJsonRpc)
Q_DECLARE_LOGGING_CATEGORY(dcLogEngine)

// Plugins

#ifdef boblight
Q_DECLARE_LOGGING_CATEGORY(dcBoblight)
#endif

Q_DECLARE_LOGGING_CATEGORY(dcCommandLauncher)
Q_DECLARE_LOGGING_CATEGORY(dcRF433)
Q_DECLARE_LOGGING_CATEGORY(dcDateTime)
Q_DECLARE_LOGGING_CATEGORY(dcEQ3)
Q_DECLARE_LOGGING_CATEGORY(dcLgSmartTv)
Q_DECLARE_LOGGING_CATEGORY(dcLircd)
Q_DECLARE_LOGGING_CATEGORY(dcMailNotification)
Q_DECLARE_LOGGING_CATEGORY(dcMock)
Q_DECLARE_LOGGING_CATEGORY(dcOpenweathermap)
Q_DECLARE_LOGGING_CATEGORY(dcPhilipsHue)
Q_DECLARE_LOGGING_CATEGORY(dcTune)
Q_DECLARE_LOGGING_CATEGORY(dcUdpCommander)
Q_DECLARE_LOGGING_CATEGORY(dcWakeOnLan)
Q_DECLARE_LOGGING_CATEGORY(dcWemo)
Q_DECLARE_LOGGING_CATEGORY(dcWifiDetector)
Q_DECLARE_LOGGING_CATEGORY(dcKodi)





#endif // LOGGINGCATEGORYS_H
