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
    \page mailnotification.html
    \title Mail Notification

    \ingroup plugins
    \ingroup services

    The mail notification plugin allows you to send a mail notification from a mail
    account by performing an \l{Action}.

    ATTENTION: The password currently will be saved as plain text in the guh settings file.
    This will be changed soon...

    \chapter Supported services
                \section2 Google Mail
                With the Google Mail Notification you can send a mail with your gmail address to a recipient. The
                username is your mail address (e.g. "chuck.norris@gmail.com"). The recipient will receive the notification
                from your gmail account.

                \section2 Yahoo Mail
                The Yahoo Mail Notification you can send a mail with your yahoo address to a recipient. The username
                is your mail address (e.g. "chuck.norris@yahoo.com"). The recipient will receive the notification
                from your yahoo account.

                \section2 Custom Mail
                With the Custom Mail Notification you can set up a custom SMTP connection. The supported authentification
                methods are ["PLAIN", "LOGIN"], the supported encryption methods are ["NONE", "SSL", "TLS"].

    \chapter Plugin properties
    Following JSON file contains the definition and the description of all available \l{DeviceClass}{DeviceClasses}
    and \l{Vendor}{Vendors} of this \l{DevicePlugin}.

    Each \l{DeviceClass} has a list of \l{ParamType}{paramTypes}, \l{ActionType}{actionTypes}, \l{StateType}{stateTypes}
    and \l{EventType}{eventTypes}. The \l{DeviceClass::CreateMethod}{createMethods} parameter describes how the \l{Device}
    will be created in the system. A device can have more than one \l{DeviceClass::CreateMethod}{CreateMethod}.
    The \l{DeviceClass::SetupMethod}{setupMethod} describes the setup method of the \l{Device}.
    The detailed implementation of each \l{DeviceClass} can be found in the source code.

    \quotefile plugins/deviceplugins/mailnotification/devicepluginmailnotification.json
*/

#include "devicepluginmailnotification.h"

#include "plugin/device.h"
#include "devicemanager.h"

#include <QDebug>
#include <QJsonDocument>
#include <QVariantMap>
#include <QDateTime>

DeviceClassId googleMailDeviceClassId = DeviceClassId("3869884a-1592-4b8f-84a7-994be18ff555");
DeviceClassId customMailDeviceClassId = DeviceClassId("f4844c97-7ca6-4349-904e-ff9749a9fe74");

ActionTypeId sendMailActionTypeId = ActionTypeId("054613b0-3666-4dad-9252-e0ebca187edc");


DevicePluginMailNotification::DevicePluginMailNotification()
{
    m_smtpClient = new SmtpClient();
}

DevicePluginMailNotification::~DevicePluginMailNotification()
{
    m_smtpClient->deleteLater();
}

DeviceManager::DeviceSetupStatus DevicePluginMailNotification::setupDevice(Device *device)
{
    Q_UNUSED(device)
    // Google mail
//    if(device->deviceClassId() == googleMailDeviceClassId){
//        m_smtpClient->setConnectionType(SmtpClient::SslConnection);
//        m_smtpClient->setAuthMethod(SmtpClient::AuthLogin);
//        m_smtpClient->setPort(465);
//        m_smtpClient->setHost("smtp.gmail.com");
//        m_smtpClient->login(device->paramValue("user").toString(), device->paramValue("password").toString());
//    }
    return DeviceManager::DeviceSetupStatusSuccess;
}

DeviceManager::HardwareResources DevicePluginMailNotification::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

DeviceManager::DeviceError DevicePluginMailNotification::executeAction(Device *device, const Action &action)
{
    qDebug() << "execute action " << sendMailActionTypeId.toString();
    if(action.actionTypeId() == sendMailActionTypeId){
        // Google mail
        if(device->deviceClassId() == googleMailDeviceClassId){
            m_smtpClient->setConnectionType(SmtpClient::SslConnection);
            m_smtpClient->setAuthMethod(SmtpClient::AuthLogin);
            m_smtpClient->setPort(465);
            m_smtpClient->setHost("smtp.gmail.com");
            m_smtpClient->setUser(device->paramValue("user").toString());
            m_smtpClient->setPassword(device->paramValue("password").toString());
            m_smtpClient->setRecipiant(device->paramValue("recipiant").toString());
        }

        if(device->deviceClassId() == customMailDeviceClassId){
            m_smtpClient->setConnectionType(SmtpClient::SslConnection);
            if(device->paramValue("auth").toString() == "PLAIN"){
                m_smtpClient->setAuthMethod(SmtpClient::AuthPlain);
            }else if(device->paramValue("auth").toString() == "LOGIN"){
                m_smtpClient->setAuthMethod(SmtpClient::AuthLogin);
            }
            m_smtpClient->setPort(device->paramValue("port").toInt());
            m_smtpClient->setHost(device->paramValue("host").toString());
            m_smtpClient->setUser(device->paramValue("user").toString());
            m_smtpClient->setPassword(device->paramValue("password").toString());
        }

        m_smtpClient->sendMail(device->paramValue("user").toString(), device->paramValue("recipient").toString(), action.param("subject").value().toString(), action.param("body").value().toString());
    }

    return DeviceManager::DeviceErrorNoError;
}
