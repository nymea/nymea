/*!
    \class DevicePlugin
    \brief This is the base class interface for device plugins.

    \ingroup devices
    \inmodule libhive

    When implementing a new plugin, start by subclassing this and implementing the following
    pure virtual methods: \l{DevicePlugin::pluginName()}, \l{DevicePlugin::pluginId()},
    \l{DevicePlugin::supportedDevices()} and \l{DevicePlugin::requiredHardware()}
*/

#include "deviceplugin.h"

#include "devicemanager.h"
#include "radio433.h"

#include <QDebug>

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
 \sa DevicePlugin::transmitData(), DevicePlugin::radioData(), DevicePlugin::hiveTimer()
 */

/*!
 \fn void DevicePlugin::radioData(QList<int> rawData)
 If the plugin has requested any radio device using \l{DevicePlugin::requiredHardware()}, this slot will
 be called when there is \a rawData available from that device.
 */

/*!
 \fn void DevicePlugin::hiveTimer()
 If the plugin has requested the timer using \l{DevicePlugin::requiredHardware()}, this slot will be called
 on timer events.
 */

/*!
 \fn void DevicePlugin::executeAction(Device *device, const Action &action)
 This will be called to actually execute actions on the hardware. The \{Device} and
 the \{Action} are contained in the \a device and \a action parameters.
 */

/*!
 \fn void DevicePlugin::emitTrigger(const Trigger &trigger)
 Emit this to produce a \l{Trigger} in the system. Usually in response to incoming data,
 such as \l{DevicePlugin::radioData()} or \l{DevicePlugin::hiveTimer()}. Find a
 configured \l{Device} from the \l{DeviceManager} and get its \l{TriggerTypes}, then
 create a \l{Trigger} complying to that \l{TriggerType} and emit it here.
 */

DevicePlugin::DevicePlugin():
    m_deviceManager(0)
{

}

DevicePlugin::~DevicePlugin()
{

}

void DevicePlugin::initPlugin(DeviceManager *deviceManager)
{
    m_deviceManager = deviceManager;
    init();
}

/*!
  Returns a map containing the plugin configuration.

  When implementing a new plugin, override this and fill in the empty configuration if your plugin requires any.
 */
QVariantMap DevicePlugin::configuration() const
{
    return QVariantMap();
}

/*!
 Will be called by the DeviceManager to set a plugin's config.

 When implementing a new plugin, override this and react to configuration changes.

 TODO: Still need to define a common format for the config.
 */
void DevicePlugin::setConfiguration(const QVariantMap &configuration)
{
    Q_UNUSED(configuration)
    qWarning() << "Plugin" << pluginName() << pluginId() << "does not support any configuration";
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
 Transmits data on the Radio433 or Radio868 devices, depending on the hardware requested by this plugin.
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

