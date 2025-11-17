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

#ifndef PLUGININFO_H
#define PLUGININFO_H

#include <QLoggingCategory>
#include <QObject>

#include "typeutils.h"

// Id definitions
PluginId pluginId = PluginId("28c7b102-3ac8-41f6-8dc0-f4787222a186");
VendorId guhVendorId = VendorId("2062d64d-3232-433c-88bc-0d33c0ba2ba6");
DeviceClassId simplebuttonDeviceClassId = DeviceClassId("c16ba02d-c982-4b45-8ca2-1945d94d8e66");
ActionTypeId simplebuttonPressActionTypeId = ActionTypeId("64c4ced5-9a1a-4858-81dd-1b5c94dba495");
EventTypeId simplebuttonPressedEventTypeId = EventTypeId("f9652210-9aed-4f38-8c19-2fd54f703fbe");

// Logging category
Q_DECLARE_LOGGING_CATEGORY(dcSimpleButton)
Q_LOGGING_CATEGORY(dcSimpleButton, "SimpleButton")

// Translation strings
const QString translations[] {
    //: The name of the plugin SimpleButton (28c7b102-3ac8-41f6-8dc0-f4787222a186)
    QT_TRANSLATE_NOOP("SimpleButton", "Simple button"), 

    //: The name of the vendor (2062d64d-3232-433c-88bc-0d33c0ba2ba6)
    QT_TRANSLATE_NOOP("SimpleButton", "nymea"), 

    //: The name of the DeviceClass (c16ba02d-c982-4b45-8ca2-1945d94d8e66)
    QT_TRANSLATE_NOOP("SimpleButton", "Simple button"), 

    //: The name of the ActionType 64c4ced5-9a1a-4858-81dd-1b5c94dba495 of deviceClass simplebutton
    QT_TRANSLATE_NOOP("SimpleButton", "press"), 

    //: The name of the EventType f9652210-9aed-4f38-8c19-2fd54f703fbe of deviceClass simplebutton
    QT_TRANSLATE_NOOP("SimpleButton", "button pressed")
};

#endif // PLUGININFO_H
