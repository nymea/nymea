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

/*!
\class DevicePlugin
\brief This is the base class interface for device plugins.

\ingroup devices
\inmodule libguh

When implementing a new plugin, start by subclassing this and implementing the following
pure virtual methods: \l{DevicePlugin::pluginName()}, \l{DevicePlugin::pluginId()},
\l{DevicePlugin::supportedDevices()} and \l{DevicePlugin::requiredHardware()}
*/

/*!
 \fn QString DevicePlugin::pluginName() const
 Return the name of the plugin. A String presented to the user.
 */

/*!
 \fn QUuid DevicePlugin::pluginId() const
 When implementing a plugin, generate a new uuid and return it here. Always return the
 same uuid and don't change it or configurations can't be matched any more.
 */

/*!
 \fn QList<DeviceClass> DevicePlugin::supportedDevices() const
 Return a list of \l{DeviceClass}{DeviceClasses} describing all the devices supported by this plugin.
 */

/*!
 \fn DeviceManager::HardwareResources DevicePlugin::requiredHardware() const
 Return flags describing the common hardware resources required by this plugin.
 \sa DevicePlugin::transmitData(), DevicePlugin::radioData(), DevicePlugin::guhTimer()
 */

/*!
 \fn void DevicePlugin::radioData(QList<int> rawData)
 If the plugin has requested any radio device using \l{DevicePlugin::requiredHardware()}, this slot will
 be called when there is \a rawData available from that device.
 */

/*!
 \fn void DevicePlugin::guhTimer()
 If the plugin has requested the timer using \l{DevicePlugin::requiredHardware()}, this slot will be called
 on timer events.
 */

/*!
 \fn void DevicePlugin::executeAction(Device *device, const Action &action)
 This will be called to actually execute actions on the hardware. The \{Device} and
 the \{Action} are contained in the \a device and \a action parameters.
 Use \l{DevicePlugin::report()} to report the result. If everything worked out,
 just return report(). Otherwise fill in the error code and a short message
 describing the offending part. E.g:
 If the action couldn't be executed because the device can't be reached (e.g. it is unplugged)
 then report the appropriate error code and give the device id as message:
 return report(DeviceManager::DeviceErrorSetupFailed, device->id());
 Keep the message short, the DeviceManager will format it for you.

 It is possible to execute actions asynchronously. You never should do anything blocking for
 a long time (e.g. wait on a network reply from the internet) but instead return
 DeviceManager::DeviceErrorAsync and continue processing in an async manner. Once
 you have the reply ready, emit actionExecutionFinished() with the appropriate parameters.

 \sa DevicePlugin::report() DevicePlugin::actionExecutionFinished()
 */

/*!
  \fn void DevicePlugin::actionExecutionFinished(const ActionId &id, DeviceManager::DeviceError status, const QString &errorMessage)
  This signal is to be emitted when you previously have returned DeviceManager::DeviceErrorAsync
  in a call of executeAction(). It is used to deliver the return value that previously has
  been omitted by filling in DeviceErrorAsync.
  */

/*!
 \fn void DevicePlugin::emitEvent(const Event &event)
 To produce a new event in the system, create a new \l{Event} and emit it with \a event.
 Usually events are emitted in response to incoming data or other other events happening,
 such as \l{DevicePlugin::radioData()} or \l{DevicePlugin::guhTimer()}. Find a configured
 \l{Device} from the \l{DeviceManager} and get its \l{EventType}{EventTypes}, then
 create a \l{Event} complying to that \l{EventType} and emit it here.
 */

/*!
  \fn void DevicePlugin::init()
  This will be called after constructing the DevicePlugin. Override this to do any
  initialisation work you need to do.
  */

#include "deviceplugin.h"

#include "devicemanager.h"
#include "hardware/radio433.h"

#include <QDebug>

/*! DevicePlugin constructor. DevicePlugins will be instantiated by the DeviceManager, its \a parent. */
DevicePlugin::DevicePlugin(QObject *parent):
    QObject(parent),
    m_deviceManager(0)
{

}

/*! Virtual destructor... */
DevicePlugin::~DevicePlugin()
{

}

/*! Override this if your plugin supports Device with DeviceClass::CreationMethodAuto.
 This will be called at startup, after the configured devices have been loaded.
 You should walk through loadedDevices and check whether all the detected devices
 are contained in there. If all the detected devices are already contained, return
 false. If instead you've found a new device which isn't known to the system yet,
 fill in the parameters of the passed device with some details that makes it possible
 for you to match this Device object with the detected hardware. After that, return true.
 The DeviceManager will then insert the device into its database and call setupDevice()
 for this device. Therefore you should not do any hardware initialisation in this state yet
 but rather wait for the subsequent setupDevice() call to set it up like in any other
 case where Device can be created.
 Returning false will cause the passed device object to be destroyed.
 If you have detected multiple new devices, just load them one by one. The DeviceManager
 will continue to call this method until you return false.
 */
bool DevicePlugin::configureAutoDevice(QList<Device*> loadedDevices, Device *device) const
{
    Q_UNUSED(loadedDevices)
    Q_UNUSED(device)
    return false;
}

DeviceManager::DeviceError DevicePlugin::discoverDevices(const DeviceClassId &deviceClassId, const QVariantMap &params) const
{
    Q_UNUSED(deviceClassId)
    Q_UNUSED(params)
    return DeviceManager::DeviceErrorCreationMethodNotSupported;
}

/*! This will be called when a new device is created. The plugin has the chance to do some setup.
    Return DeviceSetupStatusFailure if something bad happened during the setup in which case the device
    will be disabled. Return DeviceSetupStatusSuccess if everything went well. If you can't tell yet and
    need more time to set up the device (note: you should never block in this method) you can
    return DeviceSetupStatusAsync. In that case the devicemanager will wait for you to emit
    \l{deviceSetupFinished(Device *device, DeviceManager::DeviceSetupStatus status)} to report
    the status.
*/
QPair<DeviceManager::DeviceSetupStatus, QString> DevicePlugin::setupDevice(Device *device)
{
    Q_UNUSED(device)
    return reportDeviceSetup();
}

/*! This will be called when a device removed. The plugin has the chance to do some teardown.
    The device is still valid during this call, but already removed from the system.
    The device will be deleted as soon as this method returns.
*/
void DevicePlugin::deviceRemoved(Device *device)
{
    Q_UNUSED(device)
}

QList<ParamType> DevicePlugin::configurationDescription() const
{
    return QList<ParamType>();
}

/*! This will be called when the DeviceManager initializes the plugin and set up the things behind the scenes.
    When implementing a new plugin, use \l{DevicePlugin::init()} instead in order to do initialisation work. */
void DevicePlugin::initPlugin(DeviceManager *deviceManager)
{
    m_deviceManager = deviceManager;
    init();
}

/*!
  Returns a map containing the plugin configuration.

  When implementing a new plugin, override this and fill in the empty configuration if your plugin requires any.
 */
QList<Param> DevicePlugin::configuration() const
{
    return m_config;
}

/*!
 Use this to retrieve the values for your parameters. Values might not be set
 at the time when your plugin is loaded, but will be set soon after. Listen to
 configurationValueChanged() to know when something changes.
 When implementing a new plugin, specify in configurationDescription() what you want to see here.
 */
QVariant DevicePlugin::configValue(const QString &paramName) const
{
    foreach (const Param &param, m_config) {
        if (param.name() == paramName) {
            return param.value();
        }
    }
    return QVariant();
}

/*!
 Will be called by the DeviceManager to set a plugin's \a configuration.
 */
QPair<DeviceManager::DeviceError, QString> DevicePlugin::setConfiguration(const QList<Param> &configuration)
{
    foreach (const Param &param, configuration) {
        qDebug() << "setting config" << param;
        QPair<DeviceManager::DeviceError, QString> result = setConfigValue(param.name(), param.value());
        if (result.first != DeviceManager::DeviceErrorNoError) {
            return result;
        }
    }
    return report();
}

/*!
 Will be called by the DeviceManager to set a plugin's \a configuration.
 */
QPair<DeviceManager::DeviceError, QString> DevicePlugin::setConfigValue(const QString &paramName, const QVariant &value)
{
    bool found = false;
    foreach (const ParamType &paramType, configurationDescription()) {
        if (paramType.name() == paramName) {
            if (!value.canConvert(paramType.type())) {
                return report(DeviceManager::DeviceErrorInvalidParameter, QString("Wrong parameter type for param %1. Got %2. Expected %3.")
                              .arg(paramName).arg(value.toString()).arg(QVariant::typeToName(paramType.type())));
            }

            if (paramType.maxValue().isValid() && value > paramType.maxValue()) {
                return report(DeviceManager::DeviceErrorInvalidParameter, QString("Value out of range for param %1. Got %2. Max: %3.")
                              .arg(paramName).arg(value.toString()).arg(paramType.maxValue().toString()));
            }
            if (paramType.minValue().isValid() && value < paramType.minValue()) {
                return report(DeviceManager::DeviceErrorInvalidParameter, QString("Value out of range for param %1. Got: %2. Min: %3.")
                              .arg(paramName).arg(value.toString()).arg(paramType.minValue().toString()));
            }
            found = true;
            break;
        }
    }
    if (!found) {
        return report(DeviceManager::DeviceErrorInvalidParameter, QString("Invalid parameter %1.").arg(paramName));
    }
    for (int i = 0; i < m_config.count(); i++) {
        if (m_config.at(i).name() == paramName) {
            m_config[i].setValue(value);
            emit configValueChanged(paramName, value);
            return report();
        }
    }
    // Still here? need to create the param
    Param newParam(paramName, value);
    m_config.append(newParam);
    emit configValueChanged(paramName, value);
    return report();
}

/*!
 Returns a pointer to the \l{DeviceManager}.

 When implementing a plugin, use this to find the \l{Device}{Devices} you need.
 */

DeviceManager *DevicePlugin::deviceManager() const
{
    return m_deviceManager;
}

/*!
 Returns a list of all configured devices belonging to this plugin.
 */
QList<Device *> DevicePlugin::myDevices() const
{
    QList<DeviceClassId> myDeviceClassIds;
    foreach (const DeviceClass &deviceClass, supportedDevices()) {
        myDeviceClassIds.append(deviceClass.id());
    }

    QList<Device*> ret;
    foreach (Device *device, deviceManager()->configuredDevices()) {
        if (myDeviceClassIds.contains(device->deviceClassId())) {
            ret.append(device);
        }
    }
    return ret;
}

/*!
 Find a certain device from myDevices() by its params. All parameters must
 match or the device will not be found. Be prepared for nullptrs.
 */
Device *DevicePlugin::findDeviceByParams(const QList<Param> &params) const
{
    foreach (Device *device, myDevices()) {
        bool matching = true;
        foreach (const Param &param, params) {
            if (device->paramValue(param.name()) != param.value()) {
                matching = false;
            }
        }
        if (matching) {
            return device;
        }
    }
    return nullptr;
}

/*!
 Transmits data contained in \a rawData on the Radio433 or Radio868
 devices, depending on the hardware requested by this plugin.
 \sa DevicePlugin::requiredHardware()
 */
void DevicePlugin::transmitData(QList<int> rawData)
{
    switch (requiredHardware()) {
    case DeviceManager::HardwareResourceRadio433:
        deviceManager()->m_radio433->sendData(rawData);
        break;
    case DeviceManager::HardwareResourceRadio868:
        qDebug() << "Radio868 not connected yet";
        break;
    default:
        qWarning() << "Unknown harware type. Cannot send.";
    }
}

/*!
 Constructs a status report to be returned. By default (when called without
 arguments) this will report \l{DeviceManager::DeviceErrorNoError} and an
 empty message.
 Keep the message short, the DeviceManager will format it for you.
 */
QPair<DeviceManager::DeviceError, QString> DevicePlugin::report(DeviceManager::DeviceError error, const QString &message)
{
    return qMakePair<DeviceManager::DeviceError, QString>(error, message);
}

QPair<DeviceManager::DeviceSetupStatus, QString> DevicePlugin::reportDeviceSetup(DeviceManager::DeviceSetupStatus status, const QString &message)
{
    return qMakePair<DeviceManager::DeviceSetupStatus, QString>(status, message);
}
