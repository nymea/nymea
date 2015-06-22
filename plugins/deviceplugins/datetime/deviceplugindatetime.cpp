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

/*!
    \page datetime.html
    \title Time

    \ingroup plugins
    \ingroup services

    The time plugin allows you create rules based on the time, day, month, year, weekday or on weekend.

    For the setup you need to specify the continent, afterwards you can select your city/timezone. The language
    of the "month name" and "weekday name" depends on your locale settings. To have the correct time you need
    \l{https://en.wikipedia.org/wiki/Network_Time_Protocol}{ntp}.

    The weekday integer value stands for:
    \table
    \header
        \li Weekday
        \li int
    \row
        \li Monday
        \li 1
    \row
        \li Tuesday
        \li 2
    \row
        \li Wednesday
        \li 3
    \row
        \li Thursday
        \li 4
    \row
        \li Friday
        \li 5
    \row
        \li Saturday
        \li 6
    \row
        \li Sunday
        \li 7
    \endtable

    The "weekend" \l{State} will be true, if the current weekday is Saturday or Sunday, otherwise it will be false.

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    Each \l{DeviceClass} has a list of \l{ParamType}{paramTypes}, \l{ActionType}{actionTypes}, \l{StateType}{stateTypes}
    and \l{EventType}{eventTypes}. The \l{DeviceClass::CreateMethod}{createMethods} parameter describes how the \l{Device}
    will be created in the system. A device can have more than one \l{DeviceClass::CreateMethod}{CreateMethod}.
    The \l{DeviceClass::SetupMethod}{setupMethod} describes the setup method of the \l{Device}.
    The detailed implementation of each \l{DeviceClass} can be found in the source code.

    \note If a \l{StateType} has the parameter \tt{"writable": {...}}, an \l{ActionType} with the same uuid and \l{ParamType}{ParamTypes}
    will be created automatically.

    \quotefile plugins/deviceplugins/datetime/deviceplugindatetime.json
*/

#include "deviceplugindatetime.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "plugininfo.h"

#include <QDebug>

DevicePluginDateTime::DevicePluginDateTime()
{
    m_timer = new QTimer(this);
    m_timer->setInterval(1000);

    connect(m_timer, &QTimer::timeout, this, &DevicePluginDateTime::timeout);
}

DeviceManager::DeviceSetupStatus DevicePluginDateTime::setupDevice(Device *device)
{
    // check the DeviceClassId
    if(device->deviceClassId() != dateTimeDeviceClassId){
        return DeviceManager::DeviceSetupStatusFailure;
    }

    // make shore there is just one date/time
    if (myDevices().count() != 0 && myDevices().takeFirst()->id() != device->id()) {
        return DeviceManager::DeviceSetupStatusFailure;
    }

    device->setName("Time (" + device->paramValue("timezone").toString() + ")");
    m_timeZone = QTimeZone(device->paramValue("timezone").toByteArray());

    if(m_timeZone.isValid()){
        QDateTime zoneTime = QDateTime(QDate::currentDate(), QTime::currentTime(), m_timeZone).toLocalTime();
        qDebug() << zoneTime.toLocalTime().date() << zoneTime.toLocalTime().time()  << QLocale::countryToString(m_timeZone.country());
        m_timer->start();

        return DeviceManager::DeviceSetupStatusSuccess;
    }
    return DeviceManager::DeviceSetupStatusFailure;
}

DeviceManager::DeviceError DevicePluginDateTime::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(deviceClassId);

    QList<DeviceDescriptor> deviceDescriptors;
    foreach (QByteArray timeZone, QTimeZone::availableTimeZoneIds()) {
        QByteArray continent = params.paramValue("continent").toByteArray();
        if(timeZone.contains(continent)){
            DeviceDescriptor descriptor(dateTimeDeviceClassId, timeZone.right(timeZone.length() - (continent.length() + 1)), continent);
            ParamList params;
            params.append(Param("timezone", timeZone));
            descriptor.setParams(params);
            deviceDescriptors.append(descriptor);
        }
    }
    emit devicesDiscovered(dateTimeDeviceClassId, deviceDescriptors);

    return DeviceManager::DeviceErrorAsync;
}

DeviceManager::HardwareResources DevicePluginDateTime::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

DeviceManager::DeviceError DevicePluginDateTime::executeAction(Device *device, const Action &action)
{
    Q_UNUSED(device);
    Q_UNUSED(action);

    return DeviceManager::DeviceErrorNoError;
}

void DevicePluginDateTime::deviceRemoved(Device *device)
{
    Q_UNUSED(device);
    m_timer->stop();
}

void DevicePluginDateTime::timeout()
{
    QDateTime zoneTime = QDateTime(QDate::currentDate(), QTime::currentTime(), m_timeZone).toLocalTime();

    if(deviceManager()->findConfiguredDevices(dateTimeDeviceClassId).count() == 1){
        Device *device = deviceManager()->findConfiguredDevices(dateTimeDeviceClassId).first();
        device->setStateValue(minuteStateTypeId, zoneTime.time().minute());
        device->setStateValue(hourStateTypeId, zoneTime.time().hour());
        device->setStateValue(dayStateTypeId, zoneTime.date().day());
        device->setStateValue(monthStateTypeId, zoneTime.date().month());
        device->setStateValue(monthNameStateTypeId, zoneTime.date().longMonthName(zoneTime.date().month()));
        device->setStateValue(monthNameShortStateTypeId, zoneTime.date().shortMonthName(zoneTime.date().month()));
        device->setStateValue(yearStateTypeId, zoneTime.date().year());
        device->setStateValue(weekdayStateTypeId, zoneTime.date().dayOfWeek());
        device->setStateValue(weekdayNameStateTypeId, zoneTime.date().longDayName(zoneTime.date().dayOfWeek()));
        device->setStateValue(weekdayNameShortStateTypeId, zoneTime.date().shortDayName(zoneTime.date().dayOfWeek()));

        if(zoneTime.date().dayOfWeek() == 6 || zoneTime.date().dayOfWeek() == 7){
            device->setStateValue(weekendStateTypeId, true);
        }else{
            device->setStateValue(weekendStateTypeId, false);
        }
    }
}

