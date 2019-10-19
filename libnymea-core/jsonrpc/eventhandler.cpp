/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*!
    \class nymeaserver::EventHandler
    \brief This subclass of \l{JsonHandler} processes the JSON requests for the \tt Events namespace.

    \ingroup json
    \inmodule core

    This \l{JsonHandler} will be created in the \l{JsonRPCServer} and used to handle JSON-RPC requests
    for the \tt {Events} namespace of the API.

    \sa Event, JsonHandler, JsonRPCServer
*/

/*! \fn void nymeaserver::EventHandler::EventTriggered(const QVariantMap &params);
    This signal is emitted to the API notifications when an \l{Event} triggered.
    The \a params contain the map for the notification.
*/

#include "eventhandler.h"
#include "devicehandler.h"
#include "nymeacore.h"
#include "loggingcategories.h"

namespace nymeaserver {

/*! Constructs a new \l EventHandler with the given \a parent. */
EventHandler::EventHandler(QObject *parent) :
    JsonHandler(parent)
{
    // Objects
    QVariantMap event;
    event.insert("eventTypeId", enumValueName(Uuid));
    event.insert("deviceId", enumValueName(Uuid));
    event.insert("o:params", QVariantList() << objectRef("Param"));
    registerObject("Event", event);

    // Methods
    QString description; QVariantMap params; QVariantMap returns;
    description = "Get the EventType for the given eventTypeId.";
    params.insert("eventTypeId", enumValueName(Uuid));
    returns.insert("deviceError", enumRef<Device::DeviceError>());
    returns.insert("o:eventType", objectRef("EventType"));
    registerMethod("GetEventType", description, params, returns);

    // Notifications
    params.clear(); returns.clear();
    description = "Emitted whenever an Event is triggered.";
    params.insert("event", objectRef("Event"));
    registerNotification("EventTriggered", description, params);
    connect(NymeaCore::instance(), &NymeaCore::eventTriggered, this, &EventHandler::eventTriggered);
}

/*! Returns the name of the \l{EventHandler}. In this case \b Events.*/
QString EventHandler::name() const
{
    return "Events";
}

void EventHandler::eventTriggered(const Event &event)
{
    QVariantMap params;

    QVariantMap variant;
    variant.insert("eventTypeId", event.eventTypeId().toString());
    variant.insert("deviceId", event.deviceId().toString());
    QVariantList eventParams;
    foreach (const Param &param, event.params()) {
        eventParams.append(DeviceHandler::packParam(param));
    }
    variant.insert("params", eventParams);

    params.insert("event", variant);
    emit EventTriggered(params);
}

JsonReply* EventHandler::GetEventType(const QVariantMap &params) const
{
    qCDebug(dcJsonRpc) << "asked for event type" << params;
    EventTypeId eventTypeId(params.value("eventTypeId").toString());
    foreach (const DeviceClass &deviceClass, NymeaCore::instance()->deviceManager()->supportedDevices()) {
        foreach (const EventType &eventType, deviceClass.eventTypes()) {
            if (eventType.id() == eventTypeId) {
                QVariantMap data;
                data.insert("deviceError", enumValueName<Device::DeviceError>(Device::DeviceErrorNoError));
                data.insert("eventType", DeviceHandler::packEventType(eventType, deviceClass.pluginId(), params.value("locale").toLocale()));
                return createReply(data);
            }
        }
    }
    QVariantMap data;
    data.insert("deviceError", enumValueName<Device::DeviceError>(Device::DeviceErrorEventTypeNotFound));
    return createReply(data);
}

}
