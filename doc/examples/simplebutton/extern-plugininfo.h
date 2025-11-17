// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef EXTERNPLUGININFO_H
#define EXTERNPLUGININFO_H
#include "typeutils.h"
#include <QLoggingCategory>

// Id definitions
extern PluginId pluginId;
extern VendorId guhVendorId;
extern DeviceClassId simplebuttonDeviceClassId;
extern ActionTypeId simplebuttonPressActionTypeId;
extern EventTypeId simplebuttonPressedEventTypeId;

// Logging category definition
Q_DECLARE_LOGGING_CATEGORY(dcSimpleButton)

#endif // EXTERNPLUGININFO_H
