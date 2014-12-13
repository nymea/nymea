/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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
    if(deviceManager()->findConfiguredDevices(dateTimeDeviceClassId).count() != 0){
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

