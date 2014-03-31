/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

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
#include "radio433.h"

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
QVariantMap DevicePlugin::configuration() const
{
    return QVariantMap();
}

/*!
 Will be called by the DeviceManager to set a plugin's \a configuration.

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

