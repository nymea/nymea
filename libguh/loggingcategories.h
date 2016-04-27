/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.guru>                 *
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

// Include dcCoap
#include "coap/coap.h"

// Core / libguh
Q_DECLARE_LOGGING_CATEGORY(dcApplication)
Q_DECLARE_LOGGING_CATEGORY(dcDeviceManager)
Q_DECLARE_LOGGING_CATEGORY(dcTimeManager)
Q_DECLARE_LOGGING_CATEGORY(dcRuleEngine)
Q_DECLARE_LOGGING_CATEGORY(dcHardware)
Q_DECLARE_LOGGING_CATEGORY(dcConnection)
Q_DECLARE_LOGGING_CATEGORY(dcLogEngine)
Q_DECLARE_LOGGING_CATEGORY(dcTcpServer)
Q_DECLARE_LOGGING_CATEGORY(dcWebServer)
Q_DECLARE_LOGGING_CATEGORY(dcWebSocketServer)
Q_DECLARE_LOGGING_CATEGORY(dcJsonRpc)
Q_DECLARE_LOGGING_CATEGORY(dcRest)
Q_DECLARE_LOGGING_CATEGORY(dcOAuth2)

#endif // LOGGINGCATEGORYS_H
