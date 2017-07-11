/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
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

 \sa NetworkAccessManager::replyReady()
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
  \fn void DevicePlugin::configValueChanged(const ParamTypeId &paramTypeId, const QVariant &value);
  This signal is emitted when the \l{Param} with a certain \a paramTypeId of a \l{Device} configuration changed the \a value.
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
#include "guhsettings.h"
#include "hardware/radio433/radio433.h"
#include "network/upnp/upnpdiscovery.h"

#include <QDebug>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>

/*! DevicePlugin constructor. DevicePlugins will be instantiated by the DeviceManager, its \a parent. */
DevicePlugin::DevicePlugin(QObject *parent):
    QObject(parent),
    m_translator(new QTranslator(this))
{
}

/*! Virtual destructor... */
DevicePlugin::~DevicePlugin()
{

}

/*! Returns the name of this DevicePlugin. */
QString DevicePlugin::pluginName() const
{
    return translateValue(m_metaData.value("idName").toString(), m_metaData.value("name").toString());
}

/*! Returns the id of this DevicePlugin.
 *  When implementing a plugin, generate a new uuid and return it here. Always return the
 *  same uuid and don't change it or configurations can't be matched any more. */
PluginId DevicePlugin::pluginId() const
{
    return PluginId(m_metaData.value("id").toString());
}

/*! Returns the list of \l{Vendor}{Vendors} supported by this DevicePlugin. */
QList<Vendor> DevicePlugin::supportedVendors() const
{
    QList<Vendor> vendors;
    foreach (const QJsonValue &vendorJson, m_metaData.value("vendors").toArray()) {
        Vendor vendor(vendorJson.toObject().value("id").toString(), translateValue(m_metaData.value("idName").toString(), vendorJson.toObject().value("name").toString()));
        vendors.append(vendor);
    }
    return vendors;
}

/*! Return a list of \l{DeviceClass}{DeviceClasses} describing all the devices supported by this plugin.
    If a DeviceClass has an invalid parameter it will be ignored.
*/
QList<DeviceClass> DevicePlugin::supportedDevices() const
{
    QStringList missingFields = verifyFields(QStringList() << "id" << "idName" << "name" << "vendors", m_metaData);
    if (!missingFields.isEmpty()) {
        qCWarning(dcDeviceManager) << "Skipping plugin because of missing" << missingFields.join(", ") << m_metaData;
        return QList<DeviceClass>();
    }

    QList<DeviceClass> deviceClasses;
    foreach (const QJsonValue &vendorJson, m_metaData.value("vendors").toArray()) {
        bool broken = false;
        QJsonObject vendorObject = vendorJson.toObject();
        QStringList missingFields = verifyFields(QStringList() << "id" << "idName" << "name", vendorObject);
        if (!missingFields.isEmpty()) {
            qCWarning(dcDeviceManager) << "Skipping vendor because of missing" << missingFields.join(", ") << vendorObject;
            broken = true;
            break;
        }

        VendorId vendorId = vendorObject.value("id").toString();
        foreach (const QJsonValue &deviceClassJson, vendorJson.toObject().value("deviceClasses").toArray()) {
            QJsonObject deviceClassObject = deviceClassJson.toObject();
            QStringList missingFields = verifyFields(QStringList() << "id" << "idName" << "name" << "createMethods" << "paramTypes", deviceClassObject);
            if (!missingFields.isEmpty()) {
                qCWarning(dcDeviceManager) << "Skipping DeviceClass because of missing" << missingFields.join(", ") << deviceClassObject;
                broken = true;
                break;
            }

            DeviceClass deviceClass(pluginId(), vendorId, deviceClassObject.value("id").toString());
            deviceClass.setName(translateValue(m_metaData.value("idName").toString(), deviceClassObject.value("name").toString()));
            DeviceClass::CreateMethods createMethods;
            foreach (const QJsonValue &createMethodValue, deviceClassObject.value("createMethods").toArray()) {
                if (createMethodValue.toString() == "discovery") {
                    createMethods |= DeviceClass::CreateMethodDiscovery;
                } else if (createMethodValue.toString() == "auto") {
                    createMethods |= DeviceClass::CreateMethodAuto;
                } else if (createMethodValue.toString() == "user") {
                    createMethods |= DeviceClass::CreateMethodUser;
                } else {
                    qCWarning(dcDeviceManager) << "Unknown createMehtod" << createMethodValue.toString() <<
                                                  "in deviceClass " << deviceClass.name() << ". Falling back to CreateMethodUser.";
                    createMethods |= DeviceClass::CreateMethodUser;
                }
            }
            deviceClass.setCreateMethods(createMethods);
            QPair<bool, DeviceClass::DeviceIcon> deviceIconVerification = loadAndVerifyDeviceIcon(deviceClassObject.value("deviceIcon").toString());
            if (!deviceIconVerification.first) {
                broken = true;
            } else {
                deviceClass.setDeviceIcon(deviceIconVerification.second);
            }

            QPair<bool, QList<ParamType> > discoveryParamVerification = parseParamTypes(deviceClassObject.value("discoveryParamTypes").toArray());
            if (!discoveryParamVerification.first) {
                broken = true;
            } else {
                deviceClass.setDiscoveryParamTypes(discoveryParamVerification.second);
            }

            QString setupMethod = deviceClassObject.value("setupMethod").toString();
            if (setupMethod == "pushButton") {
                deviceClass.setSetupMethod(DeviceClass::SetupMethodPushButton);
            } else if (setupMethod == "displayPin") {
                deviceClass.setSetupMethod(DeviceClass::SetupMethodDisplayPin);
            } else if (setupMethod == "enterPin") {
                deviceClass.setSetupMethod(DeviceClass::SetupMethodEnterPin);
            } else if (setupMethod == "justAdd") {
                qCWarning(dcDeviceManager) << "Unknown setupMehtod" << setupMethod <<
                                              "in deviceClass " << deviceClass.name() << ". Falling back to SetupMethodJustAdd.";
                deviceClass.setSetupMethod(DeviceClass::SetupMethodJustAdd);
            }
            deviceClass.setPairingInfo(translateValue(m_metaData.value("idName").toString(), deviceClassObject.value("pairingInfo").toString()));
            QPair<bool, QList<ParamType> > paramTypesVerification = parseParamTypes(deviceClassObject.value("paramTypes").toArray());
            if (!paramTypesVerification.first) {
                broken = true;
            } else {
                deviceClass.setParamTypes(paramTypesVerification.second);
            }

            QList<DeviceClass::BasicTag> basicTags;
            foreach (const QJsonValue &basicTagJson, deviceClassObject.value("basicTags").toArray()) {
                QPair<bool, DeviceClass::BasicTag> basicTagVerification = loadAndVerifyBasicTag(basicTagJson.toString());
                if (!basicTagVerification.first) {
                    broken = true;
                    break;
                } else {
                    basicTags.append(basicTagVerification.second);
                }
            }
            deviceClass.setBasicTags(basicTags);

            QList<ActionType> actionTypes;
            QList<StateType> stateTypes;
            QList<EventType> eventTypes;

            // StateTypes
            foreach (const QJsonValue &stateTypesJson, deviceClassObject.value("stateTypes").toArray()) {
                QJsonObject st = stateTypesJson.toObject();
                QStringList missingFields = verifyFields(QStringList() << "type" << "id" << "idName" << "name" << "index" << "defaultValue" << "eventTypeName", st);
                if (!missingFields.isEmpty()) {
                    qCWarning(dcDeviceManager) << "Skipping device class" << deviceClass.name() << "because of missing" << missingFields.join(", ") << "in stateType" << st;
                    broken = true;
                    break;
                }

                QVariant::Type t = QVariant::nameToType(st.value("type").toString().toLatin1().data());
                StateType stateType(st.value("id").toString());
                stateType.setName(translateValue(m_metaData.value("idName").toString(), st.value("name").toString()));
                stateType.setIndex(st.value("index").toInt());
                stateType.setType(t);
                QPair<bool, Types::Unit> unitVerification = loadAndVerifyUnit(st.value("unit").toString());
                if (!unitVerification.first) {
                    broken = true;
                    break;
                } else {
                    stateType.setUnit(unitVerification.second);
                }
                stateType.setDefaultValue(st.value("defaultValue").toVariant());
                if (st.contains("minValue"))
                    stateType.setMinValue(st.value("minValue").toVariant());

                if (st.contains("maxValue"))
                    stateType.setMaxValue(st.value("maxValue").toVariant());

                if (st.contains("ruleRelevant"))
                    stateType.setRuleRelevant(st.value("ruleRelevant").toBool());

                if (st.contains("graphRelevant"))
                    stateType.setGraphRelevant(st.value("graphRelevant").toBool());

                if (st.contains("possibleValues")) {
                    QVariantList possibleValues;
                    foreach (const QJsonValue &possibleValueJson, st.value("possibleValues").toArray()) {
                        possibleValues.append(possibleValueJson.toVariant());
                    }
                    stateType.setPossibleValues(possibleValues);

                    if (!stateType.possibleValues().contains(stateType.defaultValue())) {
                        qCWarning(dcDeviceManager()) << QString("\"%1\" plugin:").arg(pluginName()).toLatin1().data() << QString("The given default value \"%1\" is not in the possible values of the stateType \"%2\".")
                                                        .arg(stateType.defaultValue().toString()).arg(stateType.name()).toLatin1().data();
                        broken = true;
                        break;
                    }
                }
                stateTypes.append(stateType);

                // Events for state changed
                EventType eventType(EventTypeId(stateType.id().toString()));
                if (st.contains("eventRuleRelevant"))
                    eventType.setRuleRelevant(st.value("eventRuleRelevant").toBool());

                eventType.setName(translateValue(m_metaData.value("idName").toString(), st.value("eventTypeName").toString()));
                ParamType paramType(ParamTypeId(stateType.id().toString()), translateValue(m_metaData.value("idName").toString(), st.value("name").toString()), stateType.type());
                paramType.setAllowedValues(stateType.possibleValues());
                paramType.setDefaultValue(stateType.defaultValue());
                paramType.setMinValue(stateType.minValue());
                paramType.setMaxValue(stateType.maxValue());
                paramType.setUnit(stateType.unit());
                eventType.setParamTypes(QList<ParamType>() << paramType);
                eventType.setIndex(stateType.index());
                eventTypes.append(eventType);

                // ActionTypes for writeable StateTypes
                if (st.contains("writable") && st.value("writable").toBool()) {
                    // Note: fields already checked in StateType
                    if (!st.contains("actionTypeName")) {
                        qCWarning(dcDeviceManager()) << "Missing field \"actionTypeName\" for writable StateType" << st;
                        broken = true;
                        break;
                    }

                    ActionType actionType(ActionTypeId(stateType.id().toString()));
                    actionType.setName(translateValue(m_metaData.value("idName").toString(), st.value("actionTypeName").toString()));
                    actionType.setIndex(stateType.index());
                    actionType.setParamTypes(QList<ParamType>() << paramType);
                    actionTypes.append(actionType);
                }
            }
            deviceClass.setStateTypes(stateTypes);

            // ActionTypes
            foreach (const QJsonValue &actionTypesJson, deviceClassObject.value("actionTypes").toArray()) {
                QJsonObject at = actionTypesJson.toObject();
                QStringList missingFields = verifyFields(QStringList() << "id" << "name" << "index", at);
                if (!missingFields.isEmpty()) {
                    qCWarning(dcDeviceManager) << "Skipping device class" << deviceClass.name() << "because of missing" << missingFields.join(", ") << "in actionTypes";
                    broken = true;
                    break;
                }

                ActionType actionType(at.value("id").toString());
                actionType.setName(translateValue(m_metaData.value("idName").toString(), at.value("name").toString()));
                actionType.setIndex(at.value("index").toInt());
                QPair<bool, QList<ParamType> > paramVerification = parseParamTypes(at.value("paramTypes").toArray());
                if (!paramVerification.first) {
                    broken = true;
                    break;
                } else {
                    actionType.setParamTypes(paramVerification.second);
                }

                actionTypes.append(actionType);
            }
            deviceClass.setActionTypes(actionTypes);

            // EventTypes
            foreach (const QJsonValue &eventTypesJson, deviceClassObject.value("eventTypes").toArray()) {
                QJsonObject et = eventTypesJson.toObject();
                QStringList missingFields = verifyFields(QStringList() << "id" << "name" << "index", et);
                if (!missingFields.isEmpty()) {
                    qCWarning(dcDeviceManager) << "Skipping device class" << deviceClass.name() << "because of missing" << missingFields.join(", ") << "in eventTypes";
                    broken = true;
                    break;
                }

                EventType eventType(et.value("id").toString());
                eventType.setName(translateValue(m_metaData.value("idName").toString(), translateValue(m_metaData.value("idName").toString(), et.value("name").toString())));
                eventType.setIndex(et.value("index").toInt());
                if (et.contains("ruleRelevant"))
                    eventType.setRuleRelevant(et.value("ruleRelevant").toBool());

                if (et.contains("graphRelevant"))
                    eventType.setGraphRelevant(et.value("graphRelevant").toBool());

                QPair<bool, QList<ParamType> > paramVerification = parseParamTypes(et.value("paramTypes").toArray());
                if (!paramVerification.first) {
                    broken = true;
                    break;
                } else {
                    eventType.setParamTypes(paramVerification.second);
                }
                eventTypes.append(eventType);
            }
            deviceClass.setEventTypes(eventTypes);

            // Note: keep this after the actionType / stateType / eventType parsing

            if (deviceClassObject.contains("criticalStateTypeId")) {
                StateTypeId criticalStateTypeId = StateTypeId(deviceClassObject.value("criticalStateTypeId").toString());
                if (!deviceClass.hasStateType(criticalStateTypeId)) {
                    qCWarning(dcDeviceManager) << "Skipping device class" << deviceClass.name() << ": the definend critical stateTypeId" << criticalStateTypeId.toString() << "does not match any StateType of this DeviceClass.";
                    broken = true;
                } else if (deviceClass.getStateType(criticalStateTypeId).type() != QVariant::Bool) {
                    // Make sure the critical stateType is a bool state
                    qCWarning(dcDeviceManager) << "Skipping device class" << deviceClass.name() << ": the definend critical stateTypeId" << criticalStateTypeId.toString() << "is not a bool StateType.";
                    broken = true;
                } else {
                    deviceClass.setCriticalStateTypeId(criticalStateTypeId);
                }
            }

            if (deviceClassObject.contains("primaryStateTypeId")) {
                StateTypeId primaryStateTypeId = StateTypeId(deviceClassObject.value("primaryStateTypeId").toString());
                if (!deviceClass.hasStateType(primaryStateTypeId)) {
                    qCWarning(dcDeviceManager) << "Skipping device class" << deviceClass.name() << ": the definend primary stateTypeId" << primaryStateTypeId.toString() << "does not match any StateType of this DeviceClass.";
                    broken = true;
                } else {
                    deviceClass.setPrimaryStateTypeId(primaryStateTypeId);
                }
            }

            if (deviceClassObject.contains("primaryActionTypeId")) {
                ActionTypeId primaryActionTypeId = ActionTypeId(deviceClassObject.value("primaryActionTypeId").toString());
                if (!deviceClass.hasActionType(primaryActionTypeId)) {
                    qCWarning(dcDeviceManager) << "Skipping device class" << deviceClass.name() << ": the definend primary actionTypeId" << primaryActionTypeId.toString() << "does not match any ActionType of this DeviceClass.";
                    broken = true;
                } else {
                    deviceClass.setPrimaryActionTypeId(primaryActionTypeId);
                }
            }

            QStringList interfaces;
            foreach (const QJsonValue &value, deviceClassObject.value("interfaces").toArray()) {
                QVariantMap interfaceMap = loadInterface(value.toString());
                QVariantList states = interfaceMap.value("states").toList();

                StateTypes stateTypes(deviceClass.stateTypes());
                ActionTypes actionTypes(deviceClass.actionTypes());
                EventTypes eventTypes(deviceClass.eventTypes());
                bool valid = true;
                foreach (const QVariant &stateVariant, states) {
                    StateType stateType = stateTypes.findByName(stateVariant.toMap().value("name").toString());
                    QVariantMap stateMap = stateVariant.toMap();
                    if (stateType.id().isNull()) {
                        qCWarning(dcDeviceManager) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but doesn't implement state" << stateMap.value("name").toString();
                        valid = false;
                        continue;
                    }
                    if (QVariant::nameToType(stateMap.value("type").toByteArray().data()) != stateType.type()) {
                        qCWarning(dcDeviceManager) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but state" << stateMap.value("name").toString() << "has not matching type" << stateMap.value("type").toString();
                        valid = false;
                        continue;
                    }
                    if (stateMap.contains("minimumValue")) {
                        if (stateMap.value("minimumValue").toString() == "any") {
                            if (stateType.minValue().isNull()) {
                                qCWarning(dcDeviceManager) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but state" << stateMap.value("name").toString() << "has no minimum value defined.";
                                valid = false;
                                continue;
                            }
                        } else if (stateMap.value("minimumValue") != stateType.minValue()) {
                            qCWarning(dcDeviceManager) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but state" << stateMap.value("name").toString() << "has not matching minimum value:" << stateMap.value("minimumValue") << "!=" << stateType.minValue();
                            valid = false;
                            continue;
                        }
                    }
                    if (stateMap.contains("maximumValue")) {
                        if (stateMap.value("maximumValue").toString() == "any") {
                            if (stateType.maxValue().isNull()) {
                                qCWarning(dcDeviceManager) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but state" << stateMap.value("name").toString() << "has no maximum value defined.";
                                valid = false;
                                continue;
                            }
                        } else if (stateMap.value("maximumValue") != stateType.maxValue()) {
                            qCWarning(dcDeviceManager) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but state" << stateMap.value("name").toString() << "has not matching maximum value:" << stateMap.value("maximumValue") << "!=" << stateType.minValue();
                            valid = false;
                            continue;
                        }
                    }
                    if (stateMap.contains("allowedValues") && stateMap.value("allowedValues") != stateType.possibleValues()) {
                        qCWarning(dcDeviceManager) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but state" << stateMap.value("name").toString() << "has not matching allowed values" << stateMap.value("allowedValues") << "!=" << stateType.possibleValues();
                        valid = false;
                        continue;
                    }
                    if (stateMap.contains("writable") && stateMap.value("writable").toBool() && actionTypes.findById(ActionTypeId(stateType.id().toString())).id().isNull()) {
                        qCWarning(dcDeviceManager) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but state" << stateMap.value("name").toString() << "is not writable while it should be";
                        valid = false;
                        continue;
                    }
                }
                QVariantList actions = interfaceMap.value("actions").toList();
                foreach (const QVariant &actionVariant, actions) {
                    QVariantMap actionMap = actionVariant.toMap();
                    if (actionTypes.findByName(actionMap.value("name").toString()).id().isNull()) {
                        qCWarning(dcDeviceManager) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but doesn't implement action" << actionMap.value("name").toString();
                        valid = false;
                    }
                    // TODO: check params
                }
                QVariantList events = interfaceMap.value("events").toList();
                foreach (const QVariant &eventVariant, events) {
                    QVariantMap eventMap = eventVariant.toMap();
                    if (eventTypes.findByName(eventMap.value("name").toString()).id().isNull()) {
                        qCWarning(dcDeviceManager) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but doesn't implement event" << eventMap.value("name").toString();
                        valid = false;
                    }
                    // TODO: check params
                }

                if (valid) {
                    interfaces.append(value.toString());
                }
            }
            deviceClass.setInterfaces(interfaces);

            if (!broken) {
                deviceClasses.append(deviceClass);
            } else {
                qCWarning(dcDeviceManager()) << "Skipping device class" << deviceClass.name();
            }
        }
    }
    return deviceClasses;
}

/*! Returns the translator of this \l{DevicePlugin}. */
QTranslator *DevicePlugin::translator()
{
    return m_translator;
}

/*! Returns true if the given \a locale could be set for this \l{DevicePlugin}. */
bool DevicePlugin::setLocale(const QLocale &locale)
{
    // check if there are local translations
    if (m_translator->load(locale, m_metaData.value("id").toString(), "-", QDir(QCoreApplication::applicationDirPath() + "../../translations/").absolutePath(), ".qm")) {
        qCDebug(dcDeviceManager()) << "* Load translation" << locale.name() << "for" << pluginName() << "from" << QDir(QCoreApplication::applicationDirPath() + "../../translations/").absolutePath() + "/" + m_metaData.value("id").toString() + "-" + locale.name() + ".qm";
        return true;
    }

    // otherwise use the system translations
    if (m_translator->load(locale, m_metaData.value("id").toString(), "-", GuhSettings::translationsPath(), ".qm")) {
        qCDebug(dcDeviceManager()) << "* Load translation" << locale.name() << "for" << pluginName() << "from" <<  GuhSettings::translationsPath();
        return true;
    }

    if (locale.name() != "en_US")
        qCWarning(dcDeviceManager()) << "* Could not load translation" << locale.name() << "for plugin" << pluginName();

    return false;
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

    qCWarning(dcDeviceManager) << "Plugin does not implement the display pin setup method.";

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

/*! This will be called to actually execute actions on the hardware. The \{Device} and
 * the \{Action} are contained in the \a device and \a action parameters.
 * Return the appropriate \l{DeviceManager::DeviceError}{DeviceError}.
 *
 * It is possible to execute actions asynchronously. You never should do anything blocking for
 * a long time (e.g. wait on a network reply from the internet) but instead return
 * DeviceManager::DeviceErrorAsync and continue processing in an async manner. Once
 * you have the reply ready, emit actionExecutionFinished() with the appropriate parameters.
 *
 * \sa actionExecutionFinished()
*/
DeviceManager::DeviceError DevicePlugin::executeAction(Device *device, const Action &action)
{
    Q_UNUSED(device)
    Q_UNUSED(action)
    return DeviceManager::DeviceErrorNoError;
}

/*! Returns the configuration description of this DevicePlugin as a list of \l{ParamType}{ParamTypes}. */
QList<ParamType> DevicePlugin::configurationDescription() const
{
    return m_configurationDescription;
}

/*! This will be called when the DeviceManager initializes the plugin and set up the things behind the scenes.
    When implementing a new plugin, use \l{DevicePlugin::init()} instead in order to do initialisation work. */
void DevicePlugin::initPlugin(DeviceManager *deviceManager)
{
    m_deviceManager = deviceManager;

    // parse plugin configuration params
    if (m_metaData.contains("paramTypes")) {
        QPair<bool, QList<ParamType> > paramVerification = parseParamTypes(m_metaData.value("paramTypes").toArray());
        if (paramVerification.first)
            m_configurationDescription << paramVerification.second;

    }

    init();
}

QPair<bool, QList<ParamType> > DevicePlugin::parseParamTypes(const QJsonArray &array) const
{
    QList<ParamType> paramTypes;
    foreach (const QJsonValue &paramTypesJson, array) {
        QJsonObject pt = paramTypesJson.toObject();

        // Check fields
        QStringList missingFields = verifyFields(QStringList() << "id" << "name" << "idName" << "index" << "type", pt);
        if (!missingFields.isEmpty()) {
            qCWarning(dcDeviceManager) << pluginName() << "Error parsing ParamType: missing fields" << missingFields.join(", ") << endl << pt;
            return QPair<bool, QList<ParamType> >(false, QList<ParamType>());
        }

        // Check type
        QVariant::Type t = QVariant::nameToType(pt.value("type").toString().toLatin1().data());
        if (t == QVariant::Invalid) {
            qCWarning(dcDeviceManager()) << pluginName() << QString("Invalid type %1 for param %2 in json file.")
                                            .arg(pt.value("type").toString())
                                            .arg(pt.value("name").toString()).toLatin1().data();
            return QPair<bool, QList<ParamType> >(false, QList<ParamType>());
        }

        ParamType paramType(ParamTypeId(pt.value("id").toString()), translateValue(m_metaData.value("idName").toString(), pt.value("name").toString()), t, pt.value("defaultValue").toVariant());
        paramType.setIndex(pt.value("index").toInt());

        // Set allowed values
        QVariantList allowedValues;
        foreach (const QJsonValue &allowedTypesJson, pt.value("allowedValues").toArray()) {
            allowedValues.append(allowedTypesJson.toVariant());
        }

        // Set the input type if there is any
        if (pt.contains("inputType")) {
            QPair<bool, Types::InputType> inputTypeVerification = loadAndVerifyInputType(pt.value("inputType").toString());
            if (!inputTypeVerification.first) {
                qCWarning(dcDeviceManager()) << pluginName() << QString("Invalid inputType for paramType") << pt;
                return QPair<bool, QList<ParamType> >(false, QList<ParamType>());
            } else {
                paramType.setInputType(inputTypeVerification.second);
            }
        }

        // set the unit if there is any
        if (pt.contains("unit")) {
            QPair<bool, Types::Unit> unitVerification = loadAndVerifyUnit(pt.value("unit").toString());
            if (!unitVerification.first) {
                qCWarning(dcDeviceManager()) << pluginName() << QString("Invalid unit type for paramType") << pt;
                return QPair<bool, QList<ParamType> >(false, QList<ParamType>());
            } else {
                paramType.setUnit(unitVerification.second);
            }
        }

        // set readOnly if given (default false)
        if (pt.contains("readOnly"))
            paramType.setReadOnly(pt.value("readOnly").toBool());

        paramType.setAllowedValues(allowedValues);
        paramType.setLimits(pt.value("minValue").toVariant(), pt.value("maxValue").toVariant());
        paramTypes.append(paramType);
    }

    return QPair<bool, QList<ParamType> >(true, paramTypes);
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
 Returns the config value of a \l{Param} with the given \a paramTypeId of this DevicePlugin.
 */
QVariant DevicePlugin::configValue(const ParamTypeId &paramTypeId) const
{
    return m_config.paramValue(paramTypeId);
}

/*! Will be called by the DeviceManager to set a plugin's \a configuration. */
DeviceManager::DeviceError DevicePlugin::setConfiguration(const ParamList &configuration)
{
    foreach (const Param &param, configuration) {
        qCDebug(dcDeviceManager) << "* Set plugin configuration" << param;
        DeviceManager::DeviceError result = setConfigValue(param.paramTypeId(), param.value());
        if (result != DeviceManager::DeviceErrorNoError)
            return result;

    }
    return DeviceManager::DeviceErrorNoError;
}

/*! Can be called in the DevicePlugin to set a plugin's \l{Param} with the given \a paramTypeId and \a value. */
DeviceManager::DeviceError DevicePlugin::setConfigValue(const ParamTypeId &paramTypeId, const QVariant &value)
{
    bool found = false;
    foreach (const ParamType &paramType, configurationDescription()) {
        if (paramType.id() == paramTypeId) {
            found = true;
            DeviceManager::DeviceError result = deviceManager()->verifyParam(paramType, Param(paramTypeId, value));
            if (result != DeviceManager::DeviceErrorNoError)
                return result;

            break;
        }
    }

    if (!found) {
        qCWarning(dcDeviceManager) << QString("Could not find plugin parameter with the id %1.").arg(paramTypeId.toString());
        return DeviceManager::DeviceErrorInvalidParameter;
    }

    if (m_config.hasParam(paramTypeId)) {
        if (!m_config.setParamValue(paramTypeId, value)) {
            qCWarning(dcDeviceManager()) << "Could not set param value" << value << "for param with id" << paramTypeId.toString();
            return DeviceManager::DeviceErrorInvalidParameter;
        }
    } else {
        m_config.append(Param(paramTypeId, value));
    }

    emit configValueChanged(paramTypeId, value);
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
            if (device->paramValue(param.paramTypeId()) != param.value()) {
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
 * \sa NetworkAccessManager::get()
 */
QNetworkReply *DevicePlugin::networkManagerGet(const QNetworkRequest &request)
{
    if (requiredHardware().testFlag(DeviceManager::HardwareResourceNetworkManager)) {
        return deviceManager()->m_networkManager->get(pluginId(), request);
    } else {
        qCWarning(dcDeviceManager) << "Network manager hardware resource not set for plugin" << pluginName();
    }
    return nullptr;
}
/*! Sends an HTTP POST request to the destination specified by \a request and returns a new QNetworkReply object
 * opened for reading that will contain the reply sent by the server. The contents of the \a data will be
 * uploaded to the server.
 *
 * \note The plugin has to delete the QNetworkReply with the function deleteLater().
 *
 * \sa NetworkAccessManager::post()
 */
QNetworkReply *DevicePlugin::networkManagerPost(const QNetworkRequest &request, const QByteArray &data)
{
    if (requiredHardware().testFlag(DeviceManager::HardwareResourceNetworkManager)) {
        return deviceManager()->m_networkManager->post(pluginId(), request, data);
    } else {
        qCWarning(dcDeviceManager) << "Network manager hardware resource not set for plugin" << pluginName();
    }
    return nullptr;
}

/*! Uploads the contents of \a data to the destination \a request and returnes a new QNetworkReply object that will be open for reply.
 *
 * \note The plugin has to delete the QNetworkReply with the function deleteLater().
 *
 * \sa NetworkAccessManager::put()
 */
QNetworkReply *DevicePlugin::networkManagerPut(const QNetworkRequest &request, const QByteArray &data)
{
    if (requiredHardware().testFlag(DeviceManager::HardwareResourceNetworkManager)) {
        return deviceManager()->m_networkManager->put(pluginId(), request, data);
    } else {
        qCWarning(dcDeviceManager) << "Network manager hardware resource not set for plugin" << pluginName();
    }
    return nullptr;
}

void DevicePlugin::setMetaData(const QJsonObject &metaData)
{
    m_metaData = metaData;
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
    } else {
        qCWarning(dcDeviceManager) << "UPnP discovery resource not set for plugin" << pluginName();
    }
}

/*! Returns the pointer to the central \l{QtAvahiService}{service} browser. */
QtAvahiServiceBrowser *DevicePlugin::avahiServiceBrowser() const
{
    return deviceManager()->m_avahiBrowser;
}

#ifdef BLUETOOTH_LE
bool DevicePlugin::discoverBluetooth()
{
    if(requiredHardware().testFlag(DeviceManager::HardwareResourceBluetoothLE)){
        return deviceManager()->m_bluetoothScanner->discover(pluginId());
    } else {
        qCWarning(dcDeviceManager) << "Bluetooth LE resource not set for plugin" << pluginName();
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

QString DevicePlugin::translateValue(const QString &context, const QString &string) const
{
    QString translation = m_translator->translate(context.toUtf8().constData(), string.toUtf8().constData());
    if (translation.isEmpty())
        translation = string;

    return translation;
}

QPair<bool, Types::Unit> DevicePlugin::loadAndVerifyUnit(const QString &unitString) const
{
    if (unitString.isEmpty())
        return QPair<bool, Types::Unit>(true, Types::UnitNone);

    QMetaObject metaObject = Types::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator(QString("Unit").toLatin1().data());
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);

    int enumValue = -1;
    for (int i = 0; i < metaEnum.keyCount(); i++) {
        if (QString(metaEnum.valueToKey(metaEnum.value(i))) == QString("Unit" + unitString)) {
            enumValue = metaEnum.value(i);
            break;
        }
    }

    // inform the plugin developer about the error in the plugin json file
    if (enumValue == -1) {
        qCWarning(dcDeviceManager()) << QString("\"%1\" plugin:").arg(pluginName()).toLatin1().data() << QString("Invalid unit type \"%1\" in json file.").arg(unitString).toLatin1().data();
        return QPair<bool, Types::Unit>(false, Types::UnitNone);
    }

    return QPair<bool, Types::Unit>(true, (Types::Unit)enumValue);
}

QPair<bool, Types::InputType> DevicePlugin::loadAndVerifyInputType(const QString &inputType) const
{
    if (inputType.isEmpty())
        return QPair<bool, Types::InputType>(true, Types::InputTypeNone);

    QMetaObject metaObject = Types::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator(QString("InputType").toLatin1().data());
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);

    int enumValue = -1;
    for (int i = 0; i < metaEnum.keyCount(); i++) {
        if (QString(metaEnum.valueToKey(metaEnum.value(i))) == QString("InputType" + inputType)) {
            enumValue = metaEnum.value(i);
            break;
        }
    }

    // inform the plugin developer about the error in the plugin json file
    if (enumValue == -1) {
        qCWarning(dcDeviceManager()) << QString("\"%1\" plugin:").arg(pluginName()).toLatin1().data() << QString("Invalid inputType \"%1\" in json file.").arg(inputType).toLatin1().data();
        return QPair<bool, Types::InputType>(false, Types::InputTypeNone);
    }

    return QPair<bool, Types::InputType>(true, (Types::InputType)enumValue);
}

QPair<bool, DeviceClass::BasicTag> DevicePlugin::loadAndVerifyBasicTag(const QString &basicTag) const
{
    if (basicTag.isEmpty())
        return QPair<bool, DeviceClass::BasicTag>(true, DeviceClass::BasicTagDevice);

    QMetaObject metaObject = DeviceClass::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator(QString("BasicTag").toLatin1().data());
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);

    int enumValue = -1;
    for (int i = 0; i < metaEnum.keyCount(); i++) {
        if (QString(metaEnum.valueToKey(metaEnum.value(i))) == QString("BasicTag" + basicTag)) {
            enumValue = metaEnum.value(i);
            break;
        }
    }

    // inform the plugin developer about the error in the plugin json file
    if (enumValue == -1) {
        qCWarning(dcDeviceManager()) << QString("\"%1\" plugin:").arg(pluginName()).toLatin1().data() << QString("Invalid basicTag \"%1\" in json file.").arg(basicTag).toLatin1().data();
        return QPair<bool, DeviceClass::BasicTag>(false, DeviceClass::BasicTagDevice);
    }

    return QPair<bool, DeviceClass::BasicTag>(true, (DeviceClass::BasicTag)enumValue);
}

QPair<bool, DeviceClass::DeviceIcon> DevicePlugin::loadAndVerifyDeviceIcon(const QString &deviceIcon) const
{
    if (deviceIcon.isEmpty())
        return QPair<bool, DeviceClass::DeviceIcon>(true, DeviceClass::DeviceIconNone);

    QMetaObject metaObject = DeviceClass::staticMetaObject;
    int enumIndex = metaObject.indexOfEnumerator(QString("DeviceIcon").toLatin1().data());
    QMetaEnum metaEnum = metaObject.enumerator(enumIndex);

    int enumValue = -1;
    for (int i = 0; i < metaEnum.keyCount(); i++) {
        if (QString(metaEnum.valueToKey(metaEnum.value(i))) == QString("DeviceIcon" + deviceIcon)) {
            enumValue = metaEnum.value(i);
            break;
        }
    }

    // inform the plugin developer about the error in the plugin json file
    if (enumValue == -1) {
        qCWarning(dcDeviceManager()) << QString("\"%1\" plugin:").arg(pluginName()).toLatin1().data() << QString("Invalid deviceIcon \"%1\" in json file.").arg(deviceIcon).toLatin1().data();
        return QPair<bool, DeviceClass::DeviceIcon>(false, DeviceClass::DeviceIconNone);
    }

    return QPair<bool, DeviceClass::DeviceIcon>(true, (DeviceClass::DeviceIcon)enumValue);
}

QVariantMap DevicePlugin::loadInterface(const QString &name)
{
    QFile f(QString(":/interfaces/%1.json").arg(name));
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(dcDeviceManager()) << "Failed to load interface" << name;
        return QVariantMap();
    }
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(f.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcDeviceManager) << "Cannot load interface definition for interface" << name << ":" << error.errorString();
        return QVariantMap();
    }
    QVariantMap content = jsonDoc.toVariant().toMap();
    if (content.contains("extends")) {
        QVariantMap parentContent = loadInterface(content.value("extends").toString());

        QVariantList statesList = content.value("states").toList();
        statesList.append(parentContent.value("states").toList());
        content["states"] = statesList;

        QVariantList actionsList = content.value("actions").toList();
        actionsList.append(parentContent.value("actions").toList());
        content["actions"] = actionsList;

        QVariantList eventsList = content.value("events").toList();
        eventsList.append(parentContent.value("events").toList());
        content["events"] = eventsList;
    }
    return content;
}
