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
    \class GuhCore
    \brief The main entry point for the Guh Server and the place where all the messages are dispatched.

    \ingroup core
    \inmodule server

    GuhCore is a singleton instance and the main entry point of the Guh daemon. It is responsible to
    instantiate, set up and connect all the other components.
*/

#include "guhcore.h"
#include "jsonrpcserver.h"
#include "ruleengine.h"

#include "devicemanager.h"
#include "plugin/device.h"

#include <QDebug>

GuhCore* GuhCore::s_instance = 0;

/*! Returns a pointer to the single \l{GuhCore} instance.*/
GuhCore *GuhCore::instance()
{
    if (!s_instance) {
        s_instance = new GuhCore();
    }
    return s_instance;
}

void GuhCore::destroy()
{
    delete s_instance;
    s_instance = 0;
}

/*! Returns a pointer to the \l{DeviceManager} instance owned by GuhCore.*/
DeviceManager *GuhCore::deviceManager() const
{
    return m_deviceManager;
}

/*! Returns a pointer to the \l{RuleEngine} instance owned by GuhCore.*/
RuleEngine *GuhCore::ruleEngine() const
{
    return m_ruleEngine;
}

/*! Constructs GuhCore with the given \a parent. This is private.
    Use \l{GuhCore::instance()} to access the single instance.*/
GuhCore::GuhCore(QObject *parent) :
    QObject(parent)
{
    qDebug() << "*****************************************";
    qDebug() << "* GUH version:" << GUH_VERSION_STRING << "starting up.       *";
    qDebug() << "*****************************************";

    qDebug() << "*****************************************";
    qDebug() << "* Creating Device Manager               *";
    qDebug() << "*****************************************";
    m_deviceManager = new DeviceManager(this);

    qDebug() << "*****************************************";
    qDebug() << "* Creating Rule Engine                  *";
    qDebug() << "*****************************************";
    m_ruleEngine = new RuleEngine(this);

    qDebug() << "*****************************************";
    qDebug() << "* Starting JSON RPC Server              *";
    qDebug() << "*****************************************";
    m_jsonServer = new JsonRPCServer(this);

    connect(m_deviceManager, &DeviceManager::emitEvent, this, &GuhCore::gotEvent);
}

/*! Connected to the DeviceManager's emitEvent signal. Events received in
    here will be evaluated by the \l{RuleEngine} and the according \l{Action}{Actions} are executed.*/
void GuhCore::gotEvent(const Event &event)
{
    foreach (const Action &action, m_ruleEngine->evaluateEvent(event)) {
        qDebug() << "executing action" << action.actionTypeId();
        QPair<DeviceManager::DeviceError, QString> status = m_deviceManager->executeAction(action);
        switch(status.first) {
        case DeviceManager::DeviceErrorNoError:
            break;
        case DeviceManager::DeviceErrorSetupFailed:
            qDebug() << "Error executing action. Device setup failed:" << status.second;
            break;
        case DeviceManager::DeviceErrorActionParameterError:
            qDebug() << "Error executing action. Invalid action parameter:" << status.second;
            break;
        default:
            qDebug() << "Error executing action:" << status.first << status.second;
        }
    }
}
