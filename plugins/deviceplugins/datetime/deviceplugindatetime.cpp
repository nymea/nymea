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
#include "loggingcategories.h"

DevicePluginDateTime::DevicePluginDateTime()
{
    m_timer = new QTimer(this);
    m_timer->setInterval(1000);

    qCDebug(dcDateTime) << configuration();

    m_timeZone = QTimeZone(configValue("timezone").toByteArray());

    connect(m_timer, &QTimer::timeout, this, &DevicePluginDateTime::timeout);
    connect(this, &DevicePluginDateTime::configValueChanged, this, &DevicePluginDateTime::onConfigValueChanged);
}

DeviceManager::HardwareResources DevicePluginDateTime::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

QList<ParamType> DevicePluginDateTime::configurationDescription() const
{
    QList<ParamType> params;
    ParamType timezoneParamType("timezone", QVariant::String, "Europe/Vienna");
    QList<QVariant> allowedValues;
    foreach (QByteArray timeZone, QTimeZone::availableTimeZoneIds()) {
        allowedValues.append(timeZone);
    }
    timezoneParamType.setAllowedValues(allowedValues);
    params.append(timezoneParamType);
    return params;
}

DeviceManager::DeviceSetupStatus DevicePluginDateTime::setupDevice(Device *device)
{
    // check the DeviceClassId

    device->setName("Time (" + device->paramValue("timezone").toString() + ")");
    m_timeZone = QTimeZone(device->paramValue("timezone").toByteArray());

    if(m_timeZone.isValid()){
        QDateTime zoneTime = QDateTime(QDate::currentDate(), QTime::currentTime(), m_timeZone).toLocalTime();
        qCDebug(dcDateTime) << zoneTime.toLocalTime().date() << zoneTime.toLocalTime().time()  << QLocale::countryToString(m_timeZone.country());
        m_timer->start();

        return DeviceManager::DeviceSetupStatusSuccess;
    }
    return DeviceManager::DeviceSetupStatusFailure;
}

void DevicePluginDateTime::deviceRemoved(Device *device)
{
    Q_UNUSED(device);
    m_timer->stop();
}

DeviceManager::DeviceError DevicePluginDateTime::executeAction(Device *device, const Action &action)
{
    Q_UNUSED(device);
    Q_UNUSED(action);

    return DeviceManager::DeviceErrorNoError;
}

void DevicePluginDateTime::startMonitoringAutoDevices()
{
    foreach (Device *device, myDevices()) {
        if (device->deviceClassId() == dateDeviceClassId) {
            return; // We already have a Auto Mock device... do nothing.
        }
    }

    DeviceDescriptor dateDescriptor(dateDeviceClassId, QString("Date"), QString(m_timeZone.id()));
    ParamList params;
    params.append(Param("name", m_timeZone.id()));
    dateDescriptor.setParams(params);

    emit autoDevicesAppeared(dateDeviceClassId, QList<DeviceDescriptor>() << dateDescriptor);
}

void DevicePluginDateTime::timeout()
{
    QDateTime zoneTime = QDateTime::currentDateTime().toTimeZone(m_timeZone);

    qCDebug(dcDateTime) << m_timeZone.id() << zoneTime.toString();

    if(deviceManager()->findConfiguredDevices(dateDeviceClassId).count() == 1){
        Device *device = deviceManager()->findConfiguredDevices(dateDeviceClassId).first();
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

void DevicePluginDateTime::onConfigValueChanged(const QString &paramName, const QVariant &value)
{
    Q_UNUSED(paramName)

    QTimeZone newZone = QTimeZone(value.toByteArray());
    if (newZone.isValid()) {
        m_timeZone = newZone;
        QDateTime zoneTime = QDateTime(QDate::currentDate(), QTime::currentTime(), m_timeZone);
        qCDebug(dcDateTime) << "set new time zone:" << value.toString();
        qCDebug(dcDateTime) << "current time" << zoneTime.currentDateTime().toString();
        qCDebug(dcDateTime) << "-----------------------------";
        timeout();
    } else {
        qCWarning(dcDateTime) << "could not set new timezone" << value.toString() << ". keeping old time zone:" << m_timeZone;
    }
}

