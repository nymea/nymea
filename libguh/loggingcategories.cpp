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

#include "loggingcategories.h"

Q_LOGGING_CATEGORY(dcApplication, "Application")
Q_LOGGING_CATEGORY(dcDeviceManager, "DeviceManager")
Q_LOGGING_CATEGORY(dcRuleEngine, "RuleEngine")
Q_LOGGING_CATEGORY(dcHardware, "Hardware")
Q_LOGGING_CATEGORY(dcConnection, "Connection")
Q_LOGGING_CATEGORY(dcJsonRpc, "JsonRpc")
Q_LOGGING_CATEGORY(dcLogEngine, "LogEngine")

// Plugins
#ifdef boblight
Q_LOGGING_CATEGORY(dcBoblight, "Boblight")
#endif

Q_LOGGING_CATEGORY(dcCommandLauncher, "CommandLauncher")
Q_LOGGING_CATEGORY(dcRF433, "RF433")
Q_LOGGING_CATEGORY(dcDateTime, "DateTime")
Q_LOGGING_CATEGORY(dcEQ3, "EQ-3")
Q_LOGGING_CATEGORY(dcLgSmartTv, "LgSmartTv")
Q_LOGGING_CATEGORY(dcLircd, "Lircd")
Q_LOGGING_CATEGORY(dcMailNotification, "MailNotification")
Q_LOGGING_CATEGORY(dcMock, "Mock")
Q_LOGGING_CATEGORY(dcOpenweathermap, "Openweahtermap")
Q_LOGGING_CATEGORY(dcPhilipsHue, "PhilipsHue")
Q_LOGGING_CATEGORY(dcTune, "Tune")
Q_LOGGING_CATEGORY(dcUdpCommander, "UdpCommander")
Q_LOGGING_CATEGORY(dcWakeOnLan, "WakeOnLan")
Q_LOGGING_CATEGORY(dcWemo, "Wemo")
Q_LOGGING_CATEGORY(dcWifiDetector, "WifiDetector")
