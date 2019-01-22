/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015-2018 Simon Stürz <simon.stuerz@guh.io>              *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
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
  \inmodule libnymea

*/


/*! \fn void DevicePlugin::devicesDiscovered(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &devices);
    Emit this signal when the discovery of a \a deviceClassId of this DevicePlugin is finished. The \a devices parameter describes the
    list of \l{DeviceDescriptor}{DeviceDescriptors} of all discovered \l{Device}{Devices}.
    Note: During a discovery a plugin should always return the full result set. So even if a device is already known to the system and
    a later discovery finds the device again, it should be included in the result set but the DeviceDescriptor's deviceId should be set
    to the device ID.
    \sa discoverDevices()
*/

/*! \fn void DevicePlugin::pairingFinished(const PairingTransactionId &pairingTransactionId, DeviceManager::DeviceSetupStatus status);
    This signal is emitted when the pairing of a \a pairingTransactionId is finished.
    The \a status of the  will be described as \l{DeviceManager::DeviceError}{DeviceError}.
    \sa confirmPairing()
*/

/*! \fn void DevicePlugin::deviceSetupFinished(Device *device, DeviceManager::DeviceSetupStatus status);
    This signal is emitted when the setup of a \a device in this DevicePlugin is finished. The \a status parameter describes the
    \l{DeviceManager::DeviceError}{DeviceError} that occurred.
*/

/*! \fn void DevicePlugin::configValueChanged(const ParamTypeId &paramTypeId, const QVariant &value);
    This signal is emitted when the \l{Param} with a certain \a paramTypeId of a \l{Device} configuration changed the \a value.
*/

/*! \fn void DevicePlugin::actionExecutionFinished(const ActionId &id, DeviceManager::DeviceError status)
    This signal is to be emitted when you previously have returned \l{DeviceManager}{DeviceErrorAsync}
    in a call of executeAction(). The \a id refers to the executed \l{Action}. The \a status of the \l{Action}
    execution will be described as \l{DeviceManager::DeviceError}{DeviceError}.
*/

/*! \fn void DevicePlugin::autoDevicesAppeared(const DeviceClassId &deviceClassId, const QList<DeviceDescriptor> &deviceDescriptors)
    This signal is emitted when a new \l{Device} of certain \a deviceClassId appeared. The description of the \l{Device}{Devices}
    will be in \a deviceDescriptors. This signal can only emitted from devices with the \l{DeviceClass}{CreateMethodAuto}.
*/

/*! \fn void DevicePlugin::autoDeviceDisappeared(const DeviceId &id)
    Emit this signal when a device with the given \a id and which was created by \l{DevicePlugin::autoDevicesAppeared} has been removed from the system.
    Be careful with this, as this will completely remove the device from the system and with it all the associated rules. Only
    emit this if you are sure that a device will never come back. This signal should not be emitted for child auto devices
    when the parent who created them is removed. The system will automatically remove all child devices in such a case.
*/

/*! \fn void DevicePlugin::emitEvent(const Event &event)
    To produce a new event in the system, create a new \l{Event} and emit it with \a event.
    Usually events are emitted in response to incoming data or other other events happening. Find a configured
    \l{Device} from the \l{DeviceManager} and get its \l{EventType}{EventTypes}, then
    create a \l{Event} complying to that \l{EventType} and emit it here.
*/

/*! \fn void DevicePlugin::init()
    This will be called after constructing the DevicePlugin. Override this to do any
    initialisation work you need to do.
*/

#include "deviceplugin.h"
#include "loggingcategories.h"

#include "devicemanager.h"
#include "nymeasettings.h"

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
    return m_metaData.value("name").toString();
}

/*! Returns the displayName of this DevicePlugin, to be shown to the user, translated. */
QString DevicePlugin::pluginDisplayName() const
{
    return translateValue(m_metaData.value("name").toString(), m_metaData.value("displayName").toString());
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
        Vendor vendor(vendorJson.toObject().value("id").toString(), vendorJson.toObject().value("name").toString());
        vendor.setDisplayName(translateValue(m_metaData.value("name").toString(), vendorJson.toObject().value("displayName").toString()));
        vendors.append(vendor);
    }
    return vendors;
}

/*! Return a list of \l{DeviceClass}{DeviceClasses} describing all the devices supported by this plugin.
    If a DeviceClass has an invalid parameter it will be ignored.
*/
QList<DeviceClass> DevicePlugin::supportedDevices() const
{
    return m_supportedDevices;
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
    if (m_translator->load(locale, m_metaData.value("id").toString(), "-", NymeaSettings::translationsPath(), ".qm")) {
        qCDebug(dcDeviceManager()) << "* Load translation" << locale.name() << "for" << pluginName() << "from" <<  NymeaSettings::translationsPath();
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
    Once devices are discovered, emit devicesDiscovered().
*/
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
    The device is still valid during this call, but already removed from the system.
    The device will be deleted as soon as this method returns.
*/
void DevicePlugin::deviceRemoved(Device *device)
{
    Q_UNUSED(device)
}

/*! This method will be called for \l{Device}{Devices} with the \l{DeviceClass::SetupMethodDisplayPin} right after the paring request
    with the given \a pairingTransactionId for the given \a deviceDescriptor.
*/
DeviceManager::DeviceError DevicePlugin::displayPin(const PairingTransactionId &pairingTransactionId, const DeviceDescriptor &deviceDescriptor)
{
    Q_UNUSED(pairingTransactionId)
    Q_UNUSED(deviceDescriptor)

    qCWarning(dcDeviceManager) << "Plugin does not implement the display pin setup method.";

    return DeviceManager::DeviceErrorNoError;
}

/*! Confirms the pairing of a \a deviceClassId with the given \a pairingTransactionId and \a params.
    Returns \l{DeviceManager::DeviceError}{DeviceError} to inform about the result. The optional paramerter
    \a secret contains for example the pin for \l{Device}{Devices} with the setup method \l{DeviceClass::SetupMethodDisplayPin}.
*/
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
    the \{Action} are contained in the \a device and \a action parameters.
    Return the appropriate \l{DeviceManager::DeviceError}{DeviceError}.

    It is possible to execute actions asynchronously. You never should do anything blocking for
    a long time (e.g. wait on a network reply from the internet) but instead return
    DeviceManager::DeviceErrorAsync and continue processing in an async manner. Once
    you have the reply ready, emit actionExecutionFinished() with the appropriate parameters.

    \sa actionExecutionFinished()
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
    When implementing a new plugin, use \l{DevicePlugin::init()} instead in order to do initialisation work.
    The \l{DevicePlugin::init()} method will be called once the plugin configuration has been loaded. */
void DevicePlugin::initPlugin(DeviceManager *deviceManager)
{
    m_deviceManager = deviceManager;
    loadMetaData();
}

QPair<bool, QList<ParamType> > DevicePlugin::parseParamTypes(const QJsonArray &array) const
{
    int index = 0;
    QList<ParamType> paramTypes;
    foreach (const QJsonValue &paramTypesJson, array) {
        QJsonObject pt = paramTypesJson.toObject();

        QPair<QStringList, QStringList> verificationResult = verifyFields(ParamType::typeProperties(), ParamType::mandatoryTypeProperties(), pt);

        // Check mandatory fields
        if (!verificationResult.first.isEmpty()) {
            qCWarning(dcDeviceManager()) << pluginName() << "Error parsing ParamType: missing fields:" << verificationResult.first.join(", ") << endl << pt;
            return QPair<bool, QList<ParamType> >(false, QList<ParamType>());
        }

        // Check if there are any unknown fields
        if (!verificationResult.second.isEmpty()) {
            qCWarning(dcDeviceManager()) << pluginName() << "Error parsing ParamType: unknown fields:" << verificationResult.second.join(", ") << endl << pt;
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

        ParamType paramType(ParamTypeId(pt.value("id").toString()), pt.value("name").toString(), t, pt.value("defaultValue").toVariant());
        paramType.setDisplayName(translateValue(m_metaData.value("name").toString(), pt.value("displayName").toString()));


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
        paramType.setIndex(index++);
        paramTypes.append(paramType);
    }

    return QPair<bool, QList<ParamType> >(true, paramTypes);
}

QPair<QStringList, QStringList> DevicePlugin::verifyFields(const QStringList &possibleFields, const QStringList &mandatoryFields, const QJsonObject &value) const
{
    QStringList missingFields;
    QStringList unknownFields;

    // Check if we have an unknown field
    foreach (const QString &property, value.keys()) {
        if (!possibleFields.contains(property)) {
            unknownFields << property;
        }
    }

    // Check if a mandatory field is missing
    foreach (const QString &field, mandatoryFields) {
        if (!value.contains(field)) {
            missingFields << field;
        }
    }

    return QPair<QStringList, QStringList>(missingFields, unknownFields);
}

/*! Returns a map containing the plugin configuration.
    When implementing a new plugin, override this and fill in the empty configuration if your plugin requires any.
*/
ParamList DevicePlugin::configuration() const
{
    return m_config;
}

/*! Use this to retrieve the values for your parameters. Values might not be set
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
        qCDebug(dcDeviceManager()) << "* Set plugin configuration" << param;
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
        qCWarning(dcDeviceManager()) << QString("Could not find plugin parameter with the id %1.").arg(paramTypeId.toString());
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

/*! Returns a pointer to the \l{DeviceManager}.
    When implementing a plugin, use this to find the \l{Device}{Devices} you need.
*/
DeviceManager *DevicePlugin::deviceManager() const
{
    return m_deviceManager;
}

/*! Returns a list of all configured devices belonging to this plugin. */
Devices DevicePlugin::myDevices() const
{
    QList<DeviceClassId> myDeviceClassIds;
    foreach (const DeviceClass &deviceClass, m_supportedDevices) {
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

/*! Returns the pointer to the main \l{HardwareManager} of this server. */
HardwareManager *DevicePlugin::hardwareManager() const
{
    return m_deviceManager->hardwareManager();
}

/*! Find a certain device from myDevices() by its \a params. All parameters must
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

void DevicePlugin::setMetaData(const QJsonObject &metaData)
{
    m_metaData = metaData;
}

void DevicePlugin::loadMetaData()
{

    m_configurationDescription.clear();
    m_supportedDevices.clear();

    // parse plugin configuration params
    if (m_metaData.contains("paramTypes")) {
        QPair<bool, QList<ParamType> > paramVerification = parseParamTypes(m_metaData.value("paramTypes").toArray());
        if (paramVerification.first)
            m_configurationDescription << paramVerification.second;

    }

    // Note: The DevicePlugin has no type class, so we define the json properties here
    QStringList pluginMandatoryJsonProperties = QStringList() << "id" << "name" << "displayName" << "vendors";
    QStringList pluginJsonProperties = QStringList() << "id" << "name" << "displayName" << "vendors" << "paramTypes";

    QPair<QStringList, QStringList> verificationResult = verifyFields(pluginJsonProperties, pluginMandatoryJsonProperties, m_metaData);

    // Check mandatory fields
    if (!verificationResult.first.isEmpty()) {
        qCWarning(dcDeviceManager()) << pluginName() << "Skipping plugin because of missing fields:" << verificationResult.first.join(", ") << endl << m_metaData;
        return;
    }

    // Check if there are any unknown fields
    if (!verificationResult.second.isEmpty()) {
        qCWarning(dcDeviceManager()) << pluginName() << "Skipping plugin because of unknown fields:" << verificationResult.second.join(", ") << endl << m_metaData;
        return;
    }

    // Load vendors
    foreach (const QJsonValue &vendorJson, m_metaData.value("vendors").toArray()) {
        bool broken = false;
        QJsonObject vendorObject = vendorJson.toObject();

        // Note: The Vendor has no type class, so we define the json properties here
        QStringList vendorMandatoryJsonProperties = QStringList() << "id" << "name" << "displayName" << "deviceClasses";
        QStringList vendorJsonProperties = QStringList() << "id" << "name" << "displayName" << "deviceClasses";

        QPair<QStringList, QStringList> verificationResult = verifyFields(vendorJsonProperties, vendorMandatoryJsonProperties, vendorObject);

        // Check mandatory fields
        if (!verificationResult.first.isEmpty()) {
            qCWarning(dcDeviceManager()) << pluginName() << "Skipping vendor because of missing fields:" << verificationResult.first.join(", ") << endl << vendorObject;
            broken = true;
            break;
        }

        // Check if there are any unknown fields
        if (!verificationResult.second.isEmpty()) {
            qCWarning(dcDeviceManager()) << pluginName() << "Skipping vendor because of unknown fields:" << verificationResult.second.join(", ") << endl << vendorObject;
            broken = true;
            break;
        }

        VendorId vendorId = vendorObject.value("id").toString();

        // Load deviceclasses of this vendor
        foreach (const QJsonValue &deviceClassJson, vendorJson.toObject().value("deviceClasses").toArray()) {
            QJsonObject deviceClassObject = deviceClassJson.toObject();
            QPair<QStringList, QStringList> verificationResult = verifyFields(DeviceClass::typeProperties(), DeviceClass::mandatoryTypeProperties(), deviceClassObject);

            // Check mandatory fields
            if (!verificationResult.first.isEmpty()) {
                qCWarning(dcDeviceManager()) << pluginName() << "Skipping device class because of missing fields:" << verificationResult.first.join(", ") << endl << deviceClassObject;
                broken = true;
                break;
            }

            // Check if there are any unknown fields
            if (!verificationResult.second.isEmpty()) {
                qCWarning(dcDeviceManager()) << pluginName() << "Skipping device class because of unknown fields:" << verificationResult.second.join(", ") << endl << deviceClassObject;
                broken = true;
                break;
            }

            DeviceClass deviceClass(pluginId(), vendorId, deviceClassObject.value("id").toString());
            deviceClass.setName(deviceClassObject.value("name").toString());
            deviceClass.setDisplayName(translateValue(m_metaData.value("name").toString(), deviceClassObject.value("displayName").toString()));

            // Read create methods
            DeviceClass::CreateMethods createMethods;
            if (!deviceClassObject.contains("createMethods")) {
                // Default if not specified
                createMethods |= DeviceClass::CreateMethodUser;
            } else {
                foreach (const QJsonValue &createMethodValue, deviceClassObject.value("createMethods").toArray()) {
                    if (createMethodValue.toString().toLower() == "discovery") {
                        createMethods |= DeviceClass::CreateMethodDiscovery;
                    } else if (createMethodValue.toString().toLower() == "auto") {
                        createMethods |= DeviceClass::CreateMethodAuto;
                    } else if (createMethodValue.toString().toLower() == "user") {
                        createMethods |= DeviceClass::CreateMethodUser;
                    } else {
                        qCWarning(dcDeviceManager()) << "Unknown createMehtod" << createMethodValue.toString() << "in deviceClass "
                                                     << deviceClass.name() << ". Falling back to CreateMethodUser.";
                        createMethods |= DeviceClass::CreateMethodUser;
                    }
                }
            }
            deviceClass.setCreateMethods(createMethods);

            // Read device icon
            QPair<bool, DeviceClass::DeviceIcon> deviceIconVerification = loadAndVerifyDeviceIcon(deviceClassObject.value("deviceIcon").toString());
            if (!deviceIconVerification.first) {
                broken = true;
                break;
            } else {
                deviceClass.setDeviceIcon(deviceIconVerification.second);
            }

            // Read params
            QPair<bool, QList<ParamType> > paramTypesVerification = parseParamTypes(deviceClassObject.value("paramTypes").toArray());
            if (!paramTypesVerification.first) {
                broken = true;
                break;
            } else {
                deviceClass.setParamTypes(paramTypesVerification.second);
            }

            // Read discover params
            QPair<bool, QList<ParamType> > discoveryParamVerification = parseParamTypes(deviceClassObject.value("discoveryParamTypes").toArray());
            if (!discoveryParamVerification.first) {
                broken = true;
                break;
            } else {
                deviceClass.setDiscoveryParamTypes(discoveryParamVerification.second);
            }

            // Read setup method
            DeviceClass::SetupMethod setupMethod = DeviceClass::SetupMethodJustAdd;
            if (deviceClassObject.contains("setupMethod")) {
                QString setupMethodString = deviceClassObject.value("setupMethod").toString();
                if (setupMethodString.toLower() == "pushbutton") {
                    setupMethod = DeviceClass::SetupMethodPushButton;
                } else if (setupMethodString.toLower() == "displaypin") {
                    setupMethod = DeviceClass::SetupMethodDisplayPin;
                } else if (setupMethodString.toLower() == "enterpin") {
                    setupMethod = DeviceClass::SetupMethodEnterPin;
                } else if (setupMethodString.toLower() == "justadd") {
                    setupMethod = DeviceClass::SetupMethodJustAdd;
                } else {
                    qCWarning(dcDeviceManager()) << "Unknown setupMehtod" << setupMethod << "in deviceClass"
                                               << deviceClass.name() << ". Falling back to SetupMethodJustAdd.";
                    setupMethod = DeviceClass::SetupMethodJustAdd;
                }
            }
            deviceClass.setSetupMethod(setupMethod);

            // Read pairing info
            deviceClass.setPairingInfo(translateValue(m_metaData.value("name").toString(), deviceClassObject.value("pairingInfo").toString()));

            // Read basic tags
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

            // Read StateTypes
            int index = 0;
            foreach (const QJsonValue &stateTypesJson, deviceClassObject.value("stateTypes").toArray()) {
                QJsonObject st = stateTypesJson.toObject();
                bool writableState = false;

                QPair<QStringList, QStringList> verificationResult = verifyFields(StateType::typeProperties(), StateType::mandatoryTypeProperties(), st);

                // Check mandatory fields
                if (!verificationResult.first.isEmpty()) {
                    qCWarning(dcDeviceManager()) << "Skipping device class" << deviceClass.name() << "because of missing" << verificationResult.first.join(", ") << "in stateType" << st;
                    broken = true;
                    break;
                }

                // Check if there are any unknown fields
                if (!verificationResult.second.isEmpty()) {
                    qCWarning(dcDeviceManager()) << "Skipping device class" << deviceClass.name() << "because of unknown properties" << verificationResult.second.join(", ") << "in stateType" << st;
                    broken = true;
                    break;
                }

                // If this is a writable stateType, there must be also the displayNameAction property
                if (st.contains("writable") && st.value("writable").toBool()) {
                    writableState = true;
                    if (!st.contains("displayNameAction")) {
                        qCWarning(dcDeviceManager()) << "Skipping device class" << deviceClass.name() << ". The state is writable, but does not define the displayNameAction property" << st;
                        broken = true;
                        break;
                    }
                }

                QVariant::Type t = QVariant::nameToType(st.value("type").toString().toLatin1().data());
                if (t == QVariant::Invalid) {
                    qCWarning(dcDeviceManager()) << "Invalid StateType type:" << st.value("type").toString();
                    broken = true;
                    break;
                }

                StateType stateType(st.value("id").toString());
                stateType.setName(st.value("name").toString());
                stateType.setDisplayName(translateValue(m_metaData.value("name").toString(), st.value("displayName").toString()));
                stateType.setIndex(index++);
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

                if (st.contains("cached")) {
                    stateType.setCached(st.value("cached").toBool());
                }
                stateTypes.append(stateType);

                // Events for state changed
                EventType eventType(EventTypeId(stateType.id().toString()));
                if (st.contains("eventRuleRelevant"))
                    eventType.setRuleRelevant(st.value("eventRuleRelevant").toBool());

                eventType.setName(st.value("name").toString());
                eventType.setDisplayName(translateValue(m_metaData.value("name").toString(), st.value("displayNameEvent").toString()));
                ParamType paramType(ParamTypeId(stateType.id().toString()), st.value("name").toString(), stateType.type());
                paramType.setDisplayName(translateValue(m_metaData.value("name").toString(), st.value("displayName").toString()));
                paramType.setAllowedValues(stateType.possibleValues());
                paramType.setDefaultValue(stateType.defaultValue());
                paramType.setMinValue(stateType.minValue());
                paramType.setMaxValue(stateType.maxValue());
                paramType.setUnit(stateType.unit());
                eventType.setParamTypes(QList<ParamType>() << paramType);
                eventType.setIndex(stateType.index());
                eventTypes.append(eventType);

                // ActionTypes for writeable StateTypes
                if (writableState) {
                    ActionType actionType(ActionTypeId(stateType.id().toString()));
                    actionType.setName(stateType.name());
                    actionType.setDisplayName(translateValue(m_metaData.value("name").toString(), st.value("displayNameAction").toString()));
                    actionType.setIndex(stateType.index());
                    actionType.setParamTypes(QList<ParamType>() << paramType);
                    actionTypes.append(actionType);
                }
            }
            deviceClass.setStateTypes(stateTypes);

            // ActionTypes
            index = 0;
            foreach (const QJsonValue &actionTypesJson, deviceClassObject.value("actionTypes").toArray()) {
                QJsonObject at = actionTypesJson.toObject();
                QPair<QStringList, QStringList> verificationResult = verifyFields(ActionType::typeProperties(), ActionType::mandatoryTypeProperties(), at);

                // Check mandatory fields
                if (!verificationResult.first.isEmpty()) {
                    qCWarning(dcDeviceManager()) << pluginName() << "Skipping device class" << deviceClass.name() << "because of missing" << verificationResult.first.join(", ") << "in action type:" << endl << at;
                    broken = true;
                    break;
                }

                // Check if there are any unknown fields
                if (!verificationResult.second.isEmpty()) {
                    qCWarning(dcDeviceManager()) << pluginName() << "Skipping device class" << deviceClass.name() << "because of unknown fields:" << verificationResult.second.join(", ") << "in action type:" << endl << at;
                    broken = true;
                    break;
                }

                ActionType actionType(at.value("id").toString());
                actionType.setName(at.value("name").toString());
                actionType.setDisplayName(translateValue(m_metaData.value("name").toString(), at.value("displayName").toString()));
                actionType.setIndex(index++);

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
            index = 0;
            foreach (const QJsonValue &eventTypesJson, deviceClassObject.value("eventTypes").toArray()) {
                QJsonObject et = eventTypesJson.toObject();

                QPair<QStringList, QStringList> verificationResult = verifyFields(EventType::typeProperties(), EventType::mandatoryTypeProperties(), et);

                // Check mandatory fields
                if (!verificationResult.first.isEmpty()) {
                    qCWarning(dcDeviceManager()) << pluginName() << "Skipping device class" << deviceClass.name() << "because of missing" << verificationResult.first.join(", ") << "in event type:" << endl << et;
                    broken = true;
                    break;
                }

                // Check if there are any unknown fields
                if (!verificationResult.second.isEmpty()) {
                    qCWarning(dcDeviceManager()) << pluginName() << "Skipping device class" << deviceClass.name() << "because of unknown fields:" << verificationResult.second.join(", ") << "in event type:" << endl << et;
                    broken = true;
                    break;
                }

                EventType eventType(et.value("id").toString());
                eventType.setName(et.value("name").toString());
                eventType.setDisplayName(translateValue(m_metaData.value("name").toString(), et.value("displayName").toString()));
                eventType.setIndex(index++);
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
                    qCWarning(dcDeviceManager()) << "Skipping device class" << deviceClass.name() << ": the definend critical stateTypeId" << criticalStateTypeId.toString() << "does not match any StateType of this DeviceClass.";
                    broken = true;
                } else if (deviceClass.getStateType(criticalStateTypeId).type() != QVariant::Bool) {
                    // Make sure the critical stateType is a bool state
                    qCWarning(dcDeviceManager()) << "Skipping device class" << deviceClass.name() << ": the definend critical stateTypeId" << criticalStateTypeId.toString() << "is not a bool StateType.";
                    broken = true;
                } else {
                    deviceClass.setCriticalStateTypeId(criticalStateTypeId);
                }
            }

            if (deviceClassObject.contains("primaryStateTypeId")) {
                StateTypeId primaryStateTypeId = StateTypeId(deviceClassObject.value("primaryStateTypeId").toString());
                if (!deviceClass.hasStateType(primaryStateTypeId)) {
                    qCWarning(dcDeviceManager()) << "Skipping device class" << deviceClass.name() << ": the definend primary stateTypeId" << primaryStateTypeId.toString() << "does not match any StateType of this DeviceClass.";
                    broken = true;
                } else {
                    deviceClass.setPrimaryStateTypeId(primaryStateTypeId);
                }
            }

            if (deviceClassObject.contains("primaryActionTypeId")) {
                ActionTypeId primaryActionTypeId = ActionTypeId(deviceClassObject.value("primaryActionTypeId").toString());
                if (!deviceClass.hasActionType(primaryActionTypeId)) {
                    qCWarning(dcDeviceManager()) << "Skipping device class" << deviceClass.name() << ": the definend primary actionTypeId" << primaryActionTypeId.toString() << "does not match any ActionType of this DeviceClass.";
                    broken = true;
                } else {
                    deviceClass.setPrimaryActionTypeId(primaryActionTypeId);
                }
            }

            // Read interfaces
            QStringList interfaces;
            foreach (const QJsonValue &value, deviceClassObject.value("interfaces").toArray()) {
                Interface iface = loadInterface(value.toString());

                StateTypes stateTypes(deviceClass.stateTypes());
                ActionTypes actionTypes(deviceClass.actionTypes());
                EventTypes eventTypes(deviceClass.eventTypes());

                bool valid = true;
                foreach (const StateType &ifaceStateType, iface.stateTypes()) {
                    StateType stateType = stateTypes.findByName(ifaceStateType.name());
                    if (stateType.id().isNull()) {
                        qCWarning(dcDeviceManager()) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but doesn't implement state" << ifaceStateType.name();
                        valid = false;
                        continue;
                    }
                    if (ifaceStateType.type() != stateType.type()) {
                        qCWarning(dcDeviceManager()) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but state" << stateType.name() << "has not matching type" << stateType.type() << "!=" << ifaceStateType.type();
                        valid = false;
                        continue;
                    }
                    if (ifaceStateType.minValue().isValid() && !ifaceStateType.minValue().isNull()) {
                        if (ifaceStateType.minValue().toString() == "any") {
                            if (stateType.minValue().isNull()) {
                                qCWarning(dcDeviceManager()) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but state" << stateType.name() << "has no minimum value defined.";
                                valid = false;
                                continue;
                            }
                        } else if (ifaceStateType.minValue() != stateType.minValue()) {
                            qCWarning(dcDeviceManager()) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but state" << stateType.name() << "has not matching minimum value:" << ifaceStateType.minValue() << "!=" << stateType.minValue();
                            valid = false;
                            continue;
                        }
                    }
                    if (ifaceStateType.maxValue().isValid() && !ifaceStateType.maxValue().isNull()) {
                        if (ifaceStateType.maxValue().toString() == "any") {
                            if (stateType.maxValue().isNull()) {
                                qCWarning(dcDeviceManager()) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but state" << stateType.name() << "has no maximum value defined.";
                                valid = false;
                                continue;
                            }
                        } else if (ifaceStateType.maxValue() != stateType.maxValue()) {
                            qCWarning(dcDeviceManager()) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but state" << stateType.name() << "has not matching maximum value:" << ifaceStateType.maxValue() << "!=" << stateType.minValue();
                            valid = false;
                            continue;
                        }
                    }
                    if (!ifaceStateType.possibleValues().isEmpty() && ifaceStateType.possibleValues() != stateType.possibleValues()) {
                        qCWarning(dcDeviceManager()) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but state" << stateType.name() << "has not matching allowed values" << ifaceStateType.possibleValues() << "!=" << stateType.possibleValues();
                        valid = false;
                        continue;
                    }
                }

                foreach (const ActionType &ifaceActionType, iface.actionTypes()) {
                    ActionType actionType = actionTypes.findByName(ifaceActionType.name());
                    if (actionType.id().isNull()) {
                        qCWarning(dcDeviceManager) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but doesn't implement action" << ifaceActionType.name();
                        valid = false;
                    }
                    foreach (const ParamType &ifaceActionParamType, ifaceActionType.paramTypes()) {
                        ParamType paramType = actionType.paramTypes().findByName(ifaceActionParamType.name());
                        if (!paramType.isValid()) {
                            qCWarning(dcDeviceManager) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but doesn't implement action param" << ifaceActionType.name() << ":" << ifaceActionParamType.name();
                            valid = false;
                        } else {
                            if (paramType.type() != ifaceActionParamType.type()) {
                                qCWarning(dcDeviceManager()) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but param" << paramType.name() << "is of wrong type:" << QVariant::typeToName(paramType.type()) << "expected:" << QVariant::typeToName(ifaceActionParamType.type());
                                valid = false;
                            }
                        }
                    }
                }

                foreach (const EventType &ifaceEventType, iface.eventTypes()) {
                    EventType eventType = eventTypes.findByName(ifaceEventType.name());
                    if (!eventType.isValid()) {
                        qCWarning(dcDeviceManager) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but doesn't implement event" << ifaceEventType.name();
                        valid = false;
                    }
                    foreach (const ParamType &ifaceEventParamType, ifaceEventType.paramTypes()) {
                        ParamType paramType = eventType.paramTypes().findByName(ifaceEventParamType.name());
                        if (!paramType.isValid()) {
                            qCWarning(dcDeviceManager) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but doesn't implement event param" << ifaceEventType.name() << ":" << ifaceEventParamType.name();
                            valid = false;
                        } else {
                            if (paramType.type() != ifaceEventParamType.type()) {
                                qCWarning(dcDeviceManager()) << "DeviceClass" << deviceClass.name() << "claims to implement interface" << value.toString() << "but param" << paramType.name() << "is of wrong type:" << QVariant::typeToName(paramType.type()) << "expected:" << QVariant::typeToName(ifaceEventParamType.type());
                                valid = false;
                            }
                        }
                    }
                }

                if (valid) {
                    interfaces.append(generateInterfaceParentList(value.toString()));
                }
            }
            interfaces.removeDuplicates();
            deviceClass.setInterfaces(interfaces);

            if (!broken) {
                m_supportedDevices.append(deviceClass);
            } else {
                qCWarning(dcDeviceManager()) << "Skipping device class" << deviceClass.name();
            }
        }
    }
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

Interfaces DevicePlugin::allInterfaces()
{
    Interfaces ret;
    QDir dir(":/interfaces/");
    foreach (const QFileInfo &ifaceFile, dir.entryInfoList()) {
        ret.append(loadInterface(ifaceFile.baseName()));
    }
    return ret;
}

Interface DevicePlugin::loadInterface(const QString &name)
{
    Interface iface;
    QFile f(QString(":/interfaces/%1.json").arg(name));
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(dcDeviceManager()) << "Failed to load interface" << name;
        return iface;
    }
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(f.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcDeviceManager) << "Cannot load interface definition for interface" << name << ":" << error.errorString();
        return iface;
    }
    QVariantMap content = jsonDoc.toVariant().toMap();
    if (content.contains("extends")) {
        if (!content.value("extends").toString().isEmpty()) {
            iface = loadInterface(content.value("extends").toString());
        } else if (content.value("extends").toList().count() > 0) {
            foreach (const QVariant &extendedIface, content.value("extends").toList()) {
                Interface tmp = loadInterface(extendedIface.toString());
                iface = mergeInterfaces(iface, tmp);
            }
        }
    }

    StateTypes stateTypes;
    ActionTypes actionTypes;
    EventTypes eventTypes;
    foreach (const QVariant &stateVariant, content.value("states").toList()) {
        StateType stateType(StateTypeId::fromUuid(QUuid()));
        stateType.setName(stateVariant.toMap().value("name").toString());
        stateType.setType(QVariant::nameToType(stateVariant.toMap().value("type").toByteArray()));
        stateType.setPossibleValues(stateVariant.toMap().value("allowedValues").toList());
        stateType.setMinValue(stateVariant.toMap().value("minValue"));
        stateType.setMaxValue(stateVariant.toMap().value("maxValue"));
        stateTypes.append(stateType);

        EventType stateChangeEventType(EventTypeId::fromUuid(QUuid()));
        stateChangeEventType.setName(stateType.name());
        ParamType stateChangeEventParamType;
        stateChangeEventParamType.setName(stateType.name());
        stateChangeEventParamType.setType(stateType.type());
        stateChangeEventParamType.setAllowedValues(stateType.possibleValues());
        stateChangeEventParamType.setMinValue(stateType.minValue());
        stateChangeEventParamType.setMaxValue(stateType.maxValue());
        stateChangeEventType.setParamTypes(ParamTypes() << stateChangeEventParamType);
        eventTypes.append(stateChangeEventType);

        if (stateVariant.toMap().value("writable", false).toBool()) {
            ActionType stateChangeActionType(ActionTypeId::fromUuid(QUuid()));
            stateChangeActionType.setName(stateType.name());
            stateChangeActionType.setParamTypes(ParamTypes() << stateChangeEventParamType);
            actionTypes.append(stateChangeActionType);
        }
    }

    foreach (const QVariant &actionVariant, content.value("actions").toList()) {
        ActionType actionType(ActionTypeId::fromUuid(QUuid()));
        actionType.setName(actionVariant.toMap().value("name").toString());
        ParamTypes paramTypes;
        foreach (const QVariant &actionParamVariant, actionVariant.toMap().value("params").toList()) {
            ParamType paramType;
            paramType.setName(actionParamVariant.toMap().value("name").toString());
            paramType.setType(QVariant::nameToType(actionParamVariant.toMap().value("type").toByteArray()));
            paramType.setAllowedValues(actionParamVariant.toMap().value("allowedValues").toList());
            paramType.setMinValue(actionParamVariant.toMap().value("min"));
            paramTypes.append(paramType);
        }
        actionType.setParamTypes(paramTypes);
        actionTypes.append(actionType);
    }

    foreach (const QVariant &eventVariant, content.value("events").toList()) {
        EventType eventType(EventTypeId::fromUuid(QUuid()));
        eventType.setName(eventVariant.toMap().value("name").toString());
        ParamTypes paramTypes;
        foreach (const QVariant &eventParamVariant, eventVariant.toMap().value("params").toList()) {
            ParamType paramType;
            paramType.setName(eventParamVariant.toMap().value("name").toString());
            paramType.setType(QVariant::nameToType(eventParamVariant.toMap().value("type").toByteArray()));
            paramType.setAllowedValues(eventParamVariant.toMap().value("allowedValues").toList());
            paramType.setMinValue(eventParamVariant.toMap().value("minValue"));
            paramType.setMaxValue(eventParamVariant.toMap().value("maxValue"));
            paramTypes.append(paramType);
        }
        eventType.setParamTypes(paramTypes);
        eventTypes.append(eventType);
    }

    return Interface(name, iface.actionTypes() << actionTypes, iface.eventTypes() << eventTypes, iface.stateTypes() << stateTypes);
}

Interface DevicePlugin::mergeInterfaces(const Interface &iface1, const Interface &iface2)
{
    EventTypes eventTypes = iface1.eventTypes();
    foreach (const EventType &et, iface2.eventTypes()) {
        if (eventTypes.findByName(et.name()).name().isEmpty()) {
            eventTypes.append(et);
        }
    }
    StateTypes stateTypes = iface1.stateTypes();
    foreach (const StateType &st, iface2.stateTypes()) {
        if (stateTypes.findByName(st.name()).name().isEmpty()) {
            stateTypes.append(st);
        }
    }
    ActionTypes actionTypes = iface1.actionTypes();
    foreach (const ActionType &at, iface2.actionTypes()) {
        if (actionTypes.findByName(at.name()).name().isEmpty()) {
            actionTypes.append(at);
        }
    }
    return Interface(QString(), actionTypes, eventTypes, stateTypes);
}

QStringList DevicePlugin::generateInterfaceParentList(const QString &interface)
{
    QFile f(QString(":/interfaces/%1.json").arg(interface));
    if (!f.open(QFile::ReadOnly)) {
        qCWarning(dcDeviceManager()) << "Failed to load interface" << interface;
        return QStringList();
    }
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(f.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcDeviceManager) << "Cannot load interface definition for interface" << interface << ":" << error.errorString();
        return QStringList();
    }
    QStringList ret = {interface};
    QVariantMap content = jsonDoc.toVariant().toMap();
    if (content.contains("extends")) {
        if (!content.value("extends").toString().isEmpty()) {
            ret << generateInterfaceParentList(content.value("extends").toString());
        } else if (content.value("extends").toList().count() > 0) {
            foreach (const QVariant &extendedIface, content.value("extends").toList()) {
                ret << generateInterfaceParentList(extendedIface.toString());
            }
        }
    }
    return ret;
}
