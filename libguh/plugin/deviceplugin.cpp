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

/*!
  \class DevicePlugin
  \brief This is the base class interface for device plugins.

  \ingroup devices
  \inmodule libguh

  When implementing a new plugin, start by subclassing this and implementing the following
  pure virtual method \l{DevicePlugin::requiredHardware()}
*/

/*!
 \fn DeviceManager::HardwareResources DevicePlugin::requiredHardware() const
 Return flags describing the common hardware resources required by this plugin. If you want to
 use more than one resource, you can combine them ith the OR operator.

 \sa DeviceManager::HardwareResource
 */

/*!
 \fn void DevicePlugin::radioData(const QList<int> &rawData)
 If the plugin has requested any radio device using \l{DevicePlugin::requiredHardware()}, this slot will
 be called when there is \a rawData available from that device.
 */

/*!
 \fn void DevicePlugin::guhTimer()
 If the plugin has requested the timer using \l{DevicePlugin::requiredHardware()}, this slot will be called
 on timer events.
 */

/*!
 \fn void DevicePlugin::upnpDiscoveryFinished(const QList<UpnpDeviceDescriptor> &upnpDeviceDescriptorList)
 If the plugin has requested the UPnP device list using \l{DevicePlugin::upnpDiscover()}, this slot will be called after 3
 seconds (search timeout). The \a upnpDeviceDescriptorList will contain the description of all UPnP devices available
 in the network.

 \sa upnpDiscover(), UpnpDeviceDescriptor, UpnpDiscovery::discoveryFinished()
 */

/*!
 \fn void DevicePlugin::upnpNotifyReceived(const QByteArray &notifyData)
 If a UPnP device will notify a NOTIFY message in the network, the \l{UpnpDiscovery} will catch the
 notification data and call this method with the \a notifyData.

 \note Only if if the plugin has requested the \l{DeviceManager::HardwareResourceUpnpDisovery} resource
 using \l{DevicePlugin::requiredHardware()}, this slot will be called.

 \sa UpnpDiscovery
 */

/*!
 \fn DevicePlugin::networkManagerReplyReady(QNetworkReply *reply)
 This method will be called whenever a pending network \a reply for this plugin is finished.

 \note Only if if the plugin has requested the \l{DeviceManager::HardwareResourceNetworkManager}
 resource using \l{DevicePlugin::requiredHardware()}, this slot will be called.

 \sa NetworkManager::replyReady()
 */

/*!
 \fn void DevicePlugin::executeAction(Device *device, const Action &action)
 This will be called to actually execute actions on the hardware. The \{Device} and
 the \{Action} are contained in the \a device and \a action parameters.
 Return the appropriate \l{DeviceManager::DeviceError}{DeviceError}.

 It is possible to execute actions asynchronously. You never should do anything blocking for
 a long time (e.g. wait on a network reply from the internet) but instead return
 DeviceManager::DeviceErrorAsync and continue processing in an async manner. Once
 you have the reply ready, emit actionExecutionFinished() with the appropriate parameters.

 \sa actionExecutionFinished()
*/

/*!
  \fn void DevicePlugin::devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &devices);
  This signal is emitted when the discovery of a \a deviceClassId of this DevicePlugin is finished. The \a devices parameter describes the
  list of \l{DeviceDescriptor}{DeviceDescriptors} of all discovered \l{Device}{Devices}.
  \sa discoverDevices()
*/

/*!
  \fn void DevicePlugin::pairingFinished(const PairingTransactionId &pairingTransactionId, DeviceManager::DeviceSetupStatus status);
  This signal is emitted when the pairing of a \a pairingTransactionId is finished.
  The \a status of the  will be described as \l{DeviceManager::DeviceError}{DeviceError}.
  \sa confirmPairing()
*/

/*!
  \fn void DevicePlugin::deviceSetupFinished(Device *device, DeviceManager::DeviceSetupStatus status);
  This signal is emitted when the setup of a \a device in this DevicePlugin is finished. The \a status parameter describes the
  \l{DeviceManager::DeviceError}{DeviceError} that occurred.
*/

/*!
  \fn void DevicePlugin::configValueChanged(const QString &paramName, const QVariant &value);
  This signal is emitted when the \l{Param} with a certain \a paramName of a \l{Device} configuration changed the \a value.
*/

/*!
  \fn void DevicePlugin::actionExecutionFinished(const ActionId &id, DeviceManager::DeviceError status)
  This signal is to be emitted when you previously have returned \l{DeviceManager}{DeviceErrorAsync}
  in a call of executeAction(). The \a id refers to the executed \l{Action}. The \a status of the \l{Action}
  execution will be described as \l{DeviceManager::DeviceError}{DeviceError}.
*/

/*!
  \fn void DevicePlugin::autoDevicesAppeared(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &deviceDescriptors)
   This signal is emitted when a new \l{Device} of certain \a deviceClassId appeared. The description of the \l{Device}{Devices}
   will be in \a deviceDescriptors. This signal can only emitted from devices with the \l{DeviceClass}{CreateMethodAuto}.
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
#include "loggingcategories.h"

#include "devicemanager.h"
#include "hardware/radio433/radio433.h"
#include "network/upnpdiscovery/upnpdiscovery.h"

#include <QDebug>
#include <QFileInfo>
#include <QFile>
#include <QJsonArray>

/*! DevicePlugin constructor. DevicePlugins will be instantiated by the DeviceManager, its \a parent. */
DevicePlugin::DevicePlugin(QObject *parent):
    QObject(parent)
{
}

/*! Virtual destructor... */
DevicePlugin::~DevicePlugin()
{

}

/*! Returns the name of this DevicePlugin. */
QString DevicePlugin::pluginName() const
{
    return m_metaData.value("name").toString();
}

/*! Returns the id of this DevicePlugin.
 *  When implementing a plugin, generate a new uuid and return it here. Always return the
 *  same uuid and don't change it or configurations can't be matched any more. */
PluginId DevicePlugin::pluginId() const
{
    return m_metaData.value("id").toString();
}

/*! Returns the list of \l{Vendor}{Vendors} supported by this DevicePlugin. */
QList<Vendor> DevicePlugin::supportedVendors() const
{
    QList<Vendor> vendors;
    foreach (const QJsonValue &vendorJson, m_metaData.value("vendors").toArray()) {
        Vendor vendor(vendorJson.toObject().value("id").toString(), vendorJson.toObject().value("name").toString());
        vendors.append(vendor);
    }
    return vendors;
}

/*!  Return a list of \l{DeviceClass}{DeviceClasses} describing all the devices supported by this plugin. */
QList<DeviceClass> DevicePlugin::supportedDevices() const
{
    QList<DeviceClass> deviceClasses;
    foreach (const QJsonValue &vendorJson, m_metaData.value("vendors").toArray()) {
        bool broken = false;
        VendorId vendorId = vendorJson.toObject().value("id").toString();
        foreach (const QJsonValue &deviceClassJson, vendorJson.toObject().value("deviceClasses").toArray()) {
            QJsonObject jo = deviceClassJson.toObject();
            DeviceClass deviceClass(pluginId(), vendorId, jo.value("deviceClassId").toString());
            deviceClass.setName(jo.value("name").toString());
            DeviceClass::CreateMethods createMethods;
            foreach (const QJsonValue &createMethodValue, jo.value("createMethods").toArray()) {
                if (createMethodValue.toString() == "discovery") {
                    createMethods |= DeviceClass::CreateMethodDiscovery;
                } else if (createMethodValue.toString() == "auto") {
                    createMethods |= DeviceClass::CreateMethodAuto;
                } else {
                    createMethods |= DeviceClass::CreateMethodUser;
                }
            }
            deviceClass.setCreateMethods(createMethods);

            deviceClass.setDiscoveryParamTypes(parseParamTypes(jo.value("discoveryParamTypes").toArray()));

            QString setupMethod = jo.value("setupMethod").toString();
            if (setupMethod == "pushButton") {
                deviceClass.setSetupMethod(DeviceClass::SetupMethodPushButton);
            } else if (setupMethod == "displayPin") {
                deviceClass.setSetupMethod(DeviceClass::SetupMethodDisplayPin);
            } else if (setupMethod == "enterPin") {
                deviceClass.setSetupMethod(DeviceClass::SetupMethodEnterPin);
            } else {
                deviceClass.setSetupMethod(DeviceClass::SetupMethodJustAdd);
            }
            deviceClass.setPairingInfo(jo.value("pairingInfo").toString());
            deviceClass.setParamTypes(parseParamTypes(jo.value("paramTypes").toArray()));

            QList<DeviceClass::BasicTag> basicTags;
            foreach (const QJsonValue &basicTagJson, jo.value("basicTags").toArray()) {
                basicTags.append(basicTagStringToBasicTag(basicTagJson.toString()));
            }
            deviceClass.setBasicTags(basicTags);

            QList<ActionType> actionTypes;
            QList<StateType> stateTypes;
            foreach (const QJsonValue &stateTypesJson, jo.value("stateTypes").toArray()) {
                QJsonObject st = stateTypesJson.toObject();
                QStringList missingFields = verifyFields(QStringList() << "type" << "id" << "name", st);
                if (!missingFields.isEmpty()) {
                    qCWarning(dcDeviceManager) << "Skipping device class" << deviceClass.name() << "because of missing" << missingFields.join(", ") << "in stateTypes";
                    broken = true;
                    break;
                }

                QVariant::Type t = QVariant::nameToType(st.value("type").toString().toLatin1().data());
                StateType stateType(st.value("id").toString());
                stateType.setName(st.value("name").toString());
                stateType.setType(t);
                stateType.setUnit(unitStringToUnit(st.value("unit").toString()));
                stateType.setDefaultValue(st.value("defaultValue").toVariant());
                if (st.contains("minValue"))
                    stateType.setMinValue(st.value("minValue").toVariant());

                if (st.contains("maxValue"))
                    stateType.setMaxValue(st.value("maxValue").toVariant());

                if (st.contains("possibleValues")) {
                    QVariantList possibleValues;
                    foreach (const QJsonValue &possibleValueJson, st.value("possibleValues").toArray()) {
                        possibleValues.append(possibleValueJson.toVariant());
                    }
                    stateType.setPossibleValues(possibleValues);
                }
                stateTypes.append(stateType);

                // create ActionType if this StateType is writable
                if (st.contains("writable") && st.value("writable").toBool()) {
                    // Note: fields already checked in StateType
                    ActionType actionType(ActionTypeId(stateType.id().toString()));
                    actionType.setName("set " + stateType.name());
                    ParamType paramType(stateType.name(), t, stateType.defaultValue());
                    paramType.setAllowedValues(stateType.possibleValues());
                    paramType.setUnit(stateType.unit());
                    paramType.setLimits(stateType.minValue(), stateType.maxValue());
                    actionType.setParamTypes(QList<ParamType>() << paramType);
                    actionTypes.append(actionType);
                }
            }
            deviceClass.setStateTypes(stateTypes);

            foreach (const QJsonValue &actionTypesJson, jo.value("actionTypes").toArray()) {
                QJsonObject at = actionTypesJson.toObject();
                QStringList missingFields = verifyFields(QStringList() << "id" << "name", at);
                if (!missingFields.isEmpty()) {
                    qCWarning(dcDeviceManager) << "Skipping device class" << deviceClass.name() << "because of missing" << missingFields.join(", ") << "in actionTypes";
                    broken = true;
                    break;
                }

                ActionType actionType(at.value("id").toString());
                actionType.setName(at.value("name").toString());
                actionType.setParamTypes(parseParamTypes(at.value("paramTypes").toArray()));
                actionTypes.append(actionType);
            }
            deviceClass.setActionTypes(actionTypes);

            QList<EventType> eventTypes;
            foreach (const QJsonValue &eventTypesJson, jo.value("eventTypes").toArray()) {
                QJsonObject et = eventTypesJson.toObject();
                QStringList missingFields = verifyFields(QStringList() << "id" << "name", et);
                if (!missingFields.isEmpty()) {
                    qCWarning(dcDeviceManager) << "Skipping device class" << deviceClass.name() << "because of missing" << missingFields.join(", ") << "in eventTypes";
                    broken = true;
                    break;
                }

                EventType eventType(et.value("id").toString());
                eventType.setName(et.value("name").toString());
                eventType.setParamTypes(parseParamTypes(et.value("paramTypes").toArray()));
                eventTypes.append(eventType);
            }
            deviceClass.setEventTypes(eventTypes);

            if (!broken) {
                deviceClasses.append(deviceClass);
            }
        }
    }
    return deviceClasses;
}

/*! Override this if your plugin supports Device with DeviceClass::CreationMethodAuto.
 This will be called at startup, after the configured devices have been loaded.
 This is the earliest time you should start emitting autoDevicesAppeared(). If you
 are monitoring some hardware/service for devices to appear, start monitoring now.
 If you are building the devices based on a static list, you may emit
 autoDevicesAppeard() in here.
 */
void DevicePlugin::startMonitoringAutoDevices()
{

}

/*! Reimplement this if you support a DeviceClass with createMethod \l{DeviceManager}{CreateMethodDiscovery}.
    This will be called to discover Devices for the given \a deviceClassId with the given \a params. This will always
    be an async operation. Return \l{DeviceManager}{DeviceErrorAsync} or \l{DeviceManager}{DeviceErrorNoError}
    if the discovery has been started successfully. Return an appropriate error otherwise.
    Once devices are discovered, emit devicesDiscovered(). */
DeviceManager::DeviceError DevicePlugin::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(deviceClassId)
    Q_UNUSED(params)
    return DeviceManager::DeviceErrorCreationMethodNotSupported;
}

/*! This will be called when a new device is created. The plugin has the chance to do some setup.
    Return \l{DeviceManager}{DeviceSetupStatusFailure} if something bad happened during the setup in which case the \a device
    will be disabled. Return \l{DeviceManager}{DeviceSetupStatusSuccess} if everything went well. If you can't tell yet and
    need more time to set up the \a device (note: you should never block in this method) you can
    return \l{DeviceManager}{DeviceSetupStatusAsync}. In that case the \l{DeviceManager} will wait for you to emit
    \l{DevicePlugin}{deviceSetupFinished} to report the status.
*/
DeviceManager::DeviceSetupStatus DevicePlugin::setupDevice(Device *device)
{
    Q_UNUSED(device)
    return DeviceManager::DeviceSetupStatusSuccess;
}

/*! This will be called when a new \a device was added successfully and the device setup is finished.*/
void DevicePlugin::postSetupDevice(Device *device)
{
    Q_UNUSED(device)
}

/*! This will be called when a \a device removed. The plugin has the chance to do some teardown.
 *  The device is still valid during this call, but already removed from the system.
 *  The device will be deleted as soon as this method returns.*/
void DevicePlugin::deviceRemoved(Device *device)
{
    Q_UNUSED(device)
}

/*! This method will be called for \l{Device}{Devices} with the \l{DeviceClass::SetupMethodDisplayPin} right after the paring request
 *  with the given \a pairingTransactionId for the given \a deviceDescriptor.*/
DeviceManager::DeviceError DevicePlugin::displayPin(const PairingTransactionId &pairingTransactionId, const DeviceDescriptor &deviceDescriptor)
{
    Q_UNUSED(pairingTransactionId)
    Q_UNUSED(deviceDescriptor)

    qWarning() << "Plugin does not implement the display pin setup method.";

    return DeviceManager::DeviceErrorNoError;
}

/*! Confirms the pairing of a \a deviceClassId with the given \a pairingTransactionId and \a params.
 * Returns \l{DeviceManager::DeviceError}{DeviceError} to inform about the result. The optional paramerter
 * \a secret contains for example the pin for \l{Device}{Devices} with the setup method \l{DeviceClass::SetupMethodDisplayPin}.*/
DeviceManager::DeviceSetupStatus DevicePlugin::confirmPairing(const PairingTransactionId &pairingTransactionId, const DeviceClassId &deviceClassId, const ParamList &params, const QString &secret = QString())
{
    Q_UNUSED(pairingTransactionId)
    Q_UNUSED(deviceClassId)
    Q_UNUSED(params)
    Q_UNUSED(secret)

    qCWarning(dcDeviceManager) << "Plugin does not implement pairing.";
    return DeviceManager::DeviceSetupStatusFailure;
}

/*! Returns the configuration description of this DevicePlugin as a list of \l{ParamType}{ParamTypes}. */
QList<ParamType> DevicePlugin::configurationDescription() const
{
    return QList<ParamType>();
}

/*! This will be called when the DeviceManager initializes the plugin and set up the things behind the scenes.
    When implementing a new plugin, use \l{DevicePlugin::init()} instead in order to do initialisation work. */
void DevicePlugin::initPlugin(const QJsonObject &metaData, DeviceManager *deviceManager)
{
    m_metaData = metaData;
    m_deviceManager = deviceManager;
    init();
}

QList<ParamType> DevicePlugin::parseParamTypes(const QJsonArray &array) const
{
    QList<ParamType> paramTypes;
    foreach (const QJsonValue &paramTypesJson, array) {
        QJsonObject pt = paramTypesJson.toObject();
        QVariant::Type t = QVariant::nameToType(pt.value("type").toString().toLatin1().data());
        Q_ASSERT_X(t != QVariant::Invalid,
                   pluginName().toLatin1().data(),
                   QString("Invalid type %1 for param %2 in json file.")
                   .arg(pt.value("type").toString())
                   .arg(pt.value("name").toString()).toLatin1().data());
        ParamType paramType(pt.value("name").toString(), t, pt.value("defaultValue").toVariant());

        // set allowed values
        QVariantList allowedValues;
        foreach (const QJsonValue &allowedTypesJson, pt.value("allowedValues").toArray()) {
            allowedValues.append(allowedTypesJson.toVariant());
        }

        // set the input type if there is any
        if (pt.contains("inputType")) {
            paramType.setInputType(inputTypeStringToInputType(pt.value("inputType").toString()));
        }

        // set the unit if there is any
        if (pt.contains("unit")) {
            paramType.setUnit(unitStringToUnit(pt.value("unit").toString()));
        }

        // set readOnly if given (default false)
        if (pt.contains("readOnly")) {
            paramType.setReadOnly(pt.value("readOnly").toBool());
        }
        paramType.setAllowedValues(allowedValues);
        paramType.setLimits(pt.value("minValue").toVariant(), pt.value("maxValue").toVariant());
        paramTypes.append(paramType);
    }
    return paramTypes;
}

/*!
  Returns a map containing the plugin configuration.

  When implementing a new plugin, override this and fill in the empty configuration if your plugin requires any.
 */
ParamList DevicePlugin::configuration() const
{
    return m_config;
}

/*!
 Use this to retrieve the values for your parameters. Values might not be set
 at the time when your plugin is loaded, but will be set soon after. Listen to
 configurationValueChanged() to know when something changes.
 When implementing a new plugin, specify in configurationDescription() what you want to see here.
 Returns the config value of a \l{Param} with the given \a paramName of this DevicePlugin.
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
DeviceManager::DeviceError DevicePlugin::setConfiguration(const ParamList &configuration)
{
    foreach (const Param &param, configuration) {
        qCDebug(dcDeviceManager) << "setting config" << param;
        DeviceManager::DeviceError result = setConfigValue(param.name(), param.value());
        if (result != DeviceManager::DeviceErrorNoError) {
            return result;
        }
    }
    return DeviceManager::DeviceErrorNoError;
}

/*! Will be called by the DeviceManager to set a plugin's \l{Param} with the given \a paramName and \a value. */
DeviceManager::DeviceError DevicePlugin::setConfigValue(const QString &paramName, const QVariant &value)
{
    bool found = false;
    foreach (const ParamType &paramType, configurationDescription()) {
        if (paramType.name() == paramName) {
            if (!value.canConvert(paramType.type())) {
                qCWarning(dcDeviceManager) << QString("Wrong parameter type for param %1. Got %2. Expected %3.")
                                              .arg(paramName).arg(value.toString()).arg(QVariant::typeToName(paramType.type()));
                return DeviceManager::DeviceErrorInvalidParameter;
            }

            if (paramType.maxValue().isValid() && value > paramType.maxValue()) {
                qCWarning(dcDeviceManager) << QString("Value out of range for param %1. Got %2. Max: %3.")
                                              .arg(paramName).arg(value.toString()).arg(paramType.maxValue().toString());
                return DeviceManager::DeviceErrorInvalidParameter;
            }
            if (paramType.minValue().isValid() && value < paramType.minValue()) {
                qCWarning(dcDeviceManager) << QString("Value out of range for param %1. Got: %2. Min: %3.")
                                              .arg(paramName).arg(value.toString()).arg(paramType.minValue().toString());
                return DeviceManager::DeviceErrorInvalidParameter;
            }
            found = true;
            break;
        }
    }
    if (!found) {
        qCWarning(dcDeviceManager) << QString("Invalid parameter %1.").arg(paramName);
        return DeviceManager::DeviceErrorInvalidParameter;
    }
    for (int i = 0; i < m_config.count(); i++) {
        if (m_config.at(i).name() == paramName) {
            m_config[i].setValue(value);
            emit configValueChanged(paramName, value);
            return DeviceManager::DeviceErrorNoError;
        }
    }
    // Still here? need to create the param
    Param newParam(paramName, value);
    m_config.append(newParam);
    emit configValueChanged(paramName, value);
    return DeviceManager::DeviceErrorNoError;
}

/*!
 Returns a pointer to the \l{DeviceManager}.

 When implementing a plugin, use this to find the \l{Device}{Devices} you need.
 */
DeviceManager *DevicePlugin::deviceManager() const
{
    return m_deviceManager;
}

/*! Returns a list of all configured devices belonging to this plugin. */
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
 Find a certain device from myDevices() by its \a params. All parameters must
 match or the device will not be found. Be prepared for nullptrs.
 */
Device *DevicePlugin::findDeviceByParams(const ParamList &params) const
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
 Transmits data contained in \a rawData on the \l{Radio433} devices, depending on the hardware requested by this plugin.
 Returns true if, the \a rawData with a certain \a delay (pulse length) can be sent \a repetitions times.

 \sa Radio433, requiredHardware()
 */
bool DevicePlugin::transmitData(int delay, QList<int> rawData, int repetitions)
{
    switch (requiredHardware()) {
    case DeviceManager::HardwareResourceRadio433:
        return deviceManager()->m_radio433->sendData(delay, rawData, repetitions);
    case DeviceManager::HardwareResourceRadio868:
        qCDebug(dcDeviceManager) << "Radio868 not connected yet";
        return false;
    default:
        qCWarning(dcDeviceManager) << "Unknown harware type. Cannot send.";
    }
    return false;
}
/*! Posts a request to obtain the contents of the target \a request and returns a new QNetworkReply object
 * opened for reading which emits the replyReady() signal whenever new data arrives.
 * The contents as well as associated headers will be downloaded.
 *
 * \note The plugin has to delete the QNetworkReply with the function deleteLater().
 *
 * \sa NetworkManager::get()
 */
QNetworkReply *DevicePlugin::networkManagerGet(const QNetworkRequest &request)
{
    if (requiredHardware().testFlag(DeviceManager::HardwareResourceNetworkManager)) {
        return deviceManager()->m_networkManager->get(pluginId(), request);
    } else {
        qCWarning(dcDeviceManager) << "network manager resource missing for plugin " << pluginName();
    }
    return nullptr;
}
/*! Sends an HTTP POST request to the destination specified by \a request and returns a new QNetworkReply object
 * opened for reading that will contain the reply sent by the server. The contents of the \a data will be
 * uploaded to the server.
 *
 * \note The plugin has to delete the QNetworkReply with the function deleteLater().
 *
 * \sa NetworkManager::post()
 */
QNetworkReply *DevicePlugin::networkManagerPost(const QNetworkRequest &request, const QByteArray &data)
{
    if (requiredHardware().testFlag(DeviceManager::HardwareResourceNetworkManager)) {
        return deviceManager()->m_networkManager->post(pluginId(), request, data);
    } else {
        qCWarning(dcDeviceManager) << "network manager resource missing for plugin " << pluginName();
    }
    return nullptr;
}

/*! Uploads the contents of \a data to the destination \a request and returnes a new QNetworkReply object that will be open for reply.
 *
 * \note The plugin has to delete the QNetworkReply with the function deleteLater().
 *
 * \sa NetworkManager::put()
 */
QNetworkReply *DevicePlugin::networkManagerPut(const QNetworkRequest &request, const QByteArray &data)
{
    if (requiredHardware().testFlag(DeviceManager::HardwareResourceNetworkManager)) {
        return deviceManager()->m_networkManager->put(pluginId(), request, data);
    } else {
        qCWarning(dcDeviceManager) << "network manager resource missing for plugin " << pluginName();
    }
    return nullptr;
}

/*!
 Starts a SSDP search for a certain \a searchTarget (ST). Certain UPnP devices need a special ST (i.e. "udap:rootservice"
 for LG Smart Tv's), otherwise they will not respond on the SSDP search. Each HTTP request to this device needs sometimes
 also a special \a userAgent, which will be written into the HTTP header.

 \sa DevicePlugin::requiredHardware(), DevicePlugin::upnpDiscoveryFinished()
 */
void DevicePlugin::upnpDiscover(QString searchTarget, QString userAgent)
{
    if(requiredHardware().testFlag(DeviceManager::HardwareResourceUpnpDisovery)){
        deviceManager()->m_upnpDiscovery->discoverDevices(searchTarget, userAgent, pluginId());
    }
}

#ifdef BLUETOOTH_LE
bool DevicePlugin::discoverBluetooth()
{
    if(requiredHardware().testFlag(DeviceManager::HardwareResourceBluetoothLE)){
        return deviceManager()->m_bluetoothScanner->discover(pluginId());
    }
    return false;
}
#endif

QStringList DevicePlugin::verifyFields(const QStringList &fields, const QJsonObject &value) const
{
    QStringList ret;
    foreach (const QString &field, fields) {
        if (!value.contains(field)) {
            ret << field;
        }
    }
    return ret;
}

Types::Unit DevicePlugin::unitStringToUnit(const QString &unitString) const
{
    if (unitString == QString()) {
        return Types::UnitNone;
    } else if (unitString == "Seconds") {
        return Types::UnitSeconds;
    } else if (unitString == "Minutes") {
        return Types::UnitMinutes;
    } else if (unitString == "Hours") {
        return Types::UnitHours;
    } else if (unitString == "UnixTime") {
        return Types::UnitUnixTime;
    } else if (unitString == "MeterPerSecond") {
        return Types::UnitMeterPerSecond;
    } else if (unitString == "KiloMeterPerHour") {
        return Types::UnitKiloMeterPerHour;
    } else if (unitString == "Degree") {
        return Types::UnitDegree;
    } else if (unitString == "Radiant") {
        return Types::UnitRadiant;
    } else if (unitString == "DegreeCelsius") {
        return Types::UnitDegreeCelsius;
    } else if (unitString == "DegreeKelvin") {
        return Types::UnitDegreeKelvin;
    } else if (unitString == "Mired") {
        return Types::UnitMired;
    } else if (unitString == "MilliBar") {
        return Types::UnitMilliBar;
    } else if (unitString == "Bar") {
        return Types::UnitBar;
    } else if (unitString == "Pascal") {
        return Types::UnitPascal;
    } else if (unitString == "HectoPascal") {
        return Types::UnitHectoPascal;
    } else if (unitString == "Atmosphere") {
        return Types::UnitAtmosphere;
    } else if (unitString == "Lumen") {
        return Types::UnitLumen;
    } else if (unitString == "Lux") {
        return Types::UnitLux;
    } else if (unitString == "Candela") {
        return Types::UnitCandela;
    } else if (unitString == "MilliMeter") {
        return Types::UnitMilliMeter;
    } else if (unitString == "CentiMeter") {
        return Types::UnitCentiMeter;
    } else if (unitString == "Meter") {
        return Types::UnitMeter;
    } else if (unitString == "KiloMeter") {
        return Types::UnitKiloMeter;
    } else if (unitString == "Gram") {
        return Types::UnitGram;
    } else if (unitString == "KiloGram") {
        return Types::UnitKiloGram;
    } else if (unitString == "Dezibel") {
        return Types::UnitDezibel;
    } else if (unitString == "KiloByte") {
        return Types::UnitKiloByte;
    } else if (unitString == "MegaByte") {
        return Types::UnitMegaByte;
    } else if (unitString == "GigaByte") {
        return Types::UnitGigaByte;
    } else if (unitString == "TeraByte") {
        return Types::UnitTeraByte;
    } else if (unitString == "MilliWatt") {
        return Types::UnitMilliWatt;
    } else if (unitString == "Watt") {
        return Types::UnitWatt;
    } else if (unitString == "KiloWatt") {
        return Types::UnitKiloWatt;
    } else if (unitString == "KiloWattHour") {
        return Types::UnitKiloWattHour;
    } else if (unitString == "EuroPerMegaWattHour") {
        return Types::UnitEuroPerMegaWattHour;
    } else if (unitString == "EuroCentPerKiloWattHour") {
        return Types::UnitEuroCentPerKiloWattHour;
    } else if (unitString == "Percentage") {
        return Types::UnitPercentage;
    } else if (unitString == "PartsPerMillion") {
        return Types::UnitPartsPerMillion;
    } else if (unitString == "Euro") {
        return Types::UnitEuro;
    } else if (unitString == "Dollar") {
        return Types::UnitDollar;
    } else {
        qCWarning(dcDeviceManager) << "Could not parse unit:" << unitString << "in plugin" << this->pluginName();
    }
    return Types::UnitNone;
}

Types::InputType DevicePlugin::inputTypeStringToInputType(const QString &inputType) const
{
    if (inputType == "TextLine") {
        return Types::InputTypeTextLine;
    } else if (inputType == "TextArea") {
        return Types::InputTypeTextArea;
    } else if (inputType == "Password") {
        return Types::InputTypePassword;
    } else if (inputType == "Search") {
        return Types::InputTypeSearch;
    } else if (inputType == "Mail") {
        return Types::InputTypeMail;
    } else if (inputType == "IPv4Address") {
        return Types::InputTypeIPv4Address;
    } else if (inputType == "IPv6Address") {
        return Types::InputTypeIPv6Address;
    } else if (inputType == "Url") {
        return Types::InputTypeUrl;
    } else if (inputType == "MacAddress") {
        return Types::InputTypeMacAddress;
    }
    return Types::InputTypeNone;
}

DeviceClass::BasicTag DevicePlugin::basicTagStringToBasicTag(const QString &basicTag) const
{
    if (basicTag == "Device") {
        return DeviceClass::BasicTagDevice;
    } else if (basicTag == "Service") {
        return DeviceClass::BasicTagService;
    } else if (basicTag == "Actuator") {
        return DeviceClass::BasicTagActuator;
    } else if (basicTag == "Sensor") {
        return DeviceClass::BasicTagSensor;
    } else if (basicTag == "Lighting") {
        return DeviceClass::BasicTagLighting;
    } else if (basicTag == "Energy") {
        return DeviceClass::BasicTagEnergy;
    } else if (basicTag == "Multimedia") {
        return DeviceClass::BasicTagMultimedia;
    } else if (basicTag == "Weather") {
        return DeviceClass::BasicTagWeather;
    } else if (basicTag == "Gateway") {
        return DeviceClass::BasicTagGateway;
    } else if (basicTag == "Heating") {
        return DeviceClass::BasicTagHeating;
    } else if (basicTag == "Cooling") {
        return DeviceClass::BasicTagCooling;
    } else if (basicTag == "Notification") {
        return DeviceClass::BasicTagNotification;
    } else if (basicTag == "Security") {
        return DeviceClass::BasicTagSecurity;
    } else if (basicTag == "Time") {
        return DeviceClass::BasicTagTime;
    } else if (basicTag == "Shading") {
        return DeviceClass::BasicTagShading;
    } else if (basicTag == "Appliance") {
        return DeviceClass::BasicTagAppliance;
    } else if (basicTag == "Camera") {
        return DeviceClass::BasicTagCamera;
    } else if (basicTag == "Lock") {
        return DeviceClass::BasicTagLock;
    } else {
        qCWarning(dcDeviceManager) << "Could not parse basicTag:" << basicTag << "in plugin" << this->pluginName();
    }

    return DeviceClass::BasicTagDevice;
}
