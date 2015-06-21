/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#include "eventhandler.h"
#include "guhcore.h"
#include "loggingcategories.h"

EventHandler::EventHandler(QObject *parent) :
    JsonHandler(parent)
{
    QVariantMap params;
    QVariantMap returns;

    // Notifications
    params.clear(); returns.clear();
    setDescription("EventTriggered", "Emitted whenever an Event is triggered.");
    params.insert("event", JsonTypes::eventRef());
    setParams("EventTriggered", params);

    params.clear(); returns.clear();
    setDescription("GetEventType", "Get the EventType for the given eventTypeId.");
    params.insert("eventTypeId", JsonTypes::basicTypeToString(JsonTypes::Uuid));
    setParams("GetEventType", params);
    returns.insert("deviceError", JsonTypes::deviceErrorRef());
    returns.insert("o:eventType", JsonTypes::eventTypeRef());
    setReturns("GetEventType", returns);

    connect(GuhCore::instance(), &GuhCore::eventTriggered, this, &EventHandler::eventTriggered);
}

QString EventHandler::name() const
{
    return "Events";
}

void EventHandler::eventTriggered(const Event &event)
{
    QVariantMap params;
    params.insert("event", JsonTypes::packEvent(event));
    emit EventTriggered(params);
}

JsonReply* EventHandler::GetEventType(const QVariantMap &params) const
{
    qCDebug(dcJsonRpc) << "asked for event type" << params;
    EventTypeId eventTypeId(params.value("eventTypeId").toString());
    foreach (const DeviceClass &deviceClass, GuhCore::instance()->supportedDevices()) {
        foreach (const EventType &eventType, deviceClass.eventTypes()) {
            if (eventType.id() == eventTypeId) {
                QVariantMap data = statusToReply(DeviceManager::DeviceErrorNoError);
                data.insert("eventType", JsonTypes::packEventType(eventType));
                return createReply(data);
            }
        }
    }
    return createReply(statusToReply(DeviceManager::DeviceErrorEventTypeNotFound));
}
