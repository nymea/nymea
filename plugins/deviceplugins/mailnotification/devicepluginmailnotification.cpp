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

    \note If a \l{StateType} has the parameter \tt{"writable": {...}}, an \l{ActionType} with the same uuid and \l{ParamType}{ParamTypes}
    will be created automatically.

    \quotefile plugins/deviceplugins/mailnotification/devicepluginmailnotification.json
*/

#include "devicepluginmailnotification.h"

#include "plugin/device.h"
#include "devicemanager.h"
#include "plugininfo.h"

#include <QDebug>
#include <QJsonDocument>
#include <QVariantMap>
#include <QDateTime>

DevicePluginMailNotification::DevicePluginMailNotification()
{
}

DevicePluginMailNotification::~DevicePluginMailNotification()
{
}

DeviceManager::DeviceSetupStatus DevicePluginMailNotification::setupDevice(Device *device)
{
    // Google mail
    if(device->deviceClassId() == googleMailDeviceClassId) {
        SmtpClient *smtpClient = new SmtpClient(this);
        smtpClient->setHost("smtp.gmail.com");
        smtpClient->setPort(465);
        smtpClient->setUser(device->paramValue("user").toString());
        // TODO: use cryptography to save password not as plain text
        smtpClient->setPassword(device->paramValue("password").toString());
        smtpClient->setAuthMethod(SmtpClient::AuthMethodLogin);
        smtpClient->setEncryptionType(SmtpClient::EncryptionTypeSSL);
        smtpClient->setSender(device->paramValue("user").toString());
        smtpClient->setRecipient(device->paramValue("recipient").toString());

        connect(smtpClient, &SmtpClient::testLoginFinished, this, &DevicePluginMailNotification::testLoginFinished);
        connect(smtpClient, &SmtpClient::sendMailFinished, this, &DevicePluginMailNotification::sendMailFinished);
        m_clients.insert(smtpClient,device);

        smtpClient->testLogin();

        return DeviceManager::DeviceSetupStatusAsync;
    }
    // Yahoo mail
    if(device->deviceClassId() == yahooMailDeviceClassId) {
        SmtpClient *smtpClient = new SmtpClient(this);
        smtpClient->setHost("smtp.mail.yahoo.com");
        smtpClient->setPort(465);
        smtpClient->setUser(device->paramValue("user").toString());
        // TODO: use cryptography to save password not as plain text
        smtpClient->setPassword(device->paramValue("password").toString());
        smtpClient->setAuthMethod(SmtpClient::AuthMethodLogin);
        smtpClient->setEncryptionType(SmtpClient::EncryptionTypeSSL);
        smtpClient->setSender(device->paramValue("user").toString());
        smtpClient->setRecipient(device->paramValue("recipient").toString());

        connect(smtpClient, &SmtpClient::testLoginFinished, this, &DevicePluginMailNotification::testLoginFinished);
        connect(smtpClient, &SmtpClient::sendMailFinished, this, &DevicePluginMailNotification::sendMailFinished);
        m_clients.insert(smtpClient,device);

        smtpClient->testLogin();

        return DeviceManager::DeviceSetupStatusAsync;
    }
    // Custom mail
    if(device->deviceClassId() == customMailDeviceClassId) {
        SmtpClient *smtpClient = new SmtpClient(this);
        smtpClient->setHost(device->paramValue("SMTP server").toString());
        smtpClient->setPort(device->paramValue("port").toInt());
        smtpClient->setUser(device->paramValue("user").toString());

        // TODO: use cryptography to save password not as plain text
        smtpClient->setPassword(device->paramValue("password").toString());

        if(device->paramValue("authentification").toString() == "PLAIN") {
            smtpClient->setAuthMethod(SmtpClient::AuthMethodPlain);
        } else if(device->paramValue("authentification").toString() == "LOGIN") {
            smtpClient->setAuthMethod(SmtpClient::AuthMethodLogin);
        } else {
            return DeviceManager::DeviceSetupStatusFailure;
        }

        if(device->paramValue("encryption").toString() == "NONE") {
            smtpClient->setEncryptionType(SmtpClient::EncryptionTypeNone);
        } else if(device->paramValue("encryption").toString() == "SSL") {
            smtpClient->setEncryptionType(SmtpClient::EncryptionTypeSSL);
        } else if(device->paramValue("encryption").toString() == "TLS") {
            smtpClient->setEncryptionType(SmtpClient::EncryptionTypeTLS);
        } else {
            return DeviceManager::DeviceSetupStatusFailure;
        }

        smtpClient->setRecipient(device->paramValue("recipient").toString());
        smtpClient->setSender(device->paramValue("sender mail").toString());

        connect(smtpClient, &SmtpClient::testLoginFinished, this, &DevicePluginMailNotification::testLoginFinished);
        connect(smtpClient, &SmtpClient::sendMailFinished, this, &DevicePluginMailNotification::sendMailFinished);
        m_clients.insert(smtpClient,device);

        smtpClient->testLogin();

        return DeviceManager::DeviceSetupStatusAsync;
    }
    return DeviceManager::DeviceSetupStatusFailure;
}

DeviceManager::HardwareResources DevicePluginMailNotification::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

DeviceManager::DeviceError DevicePluginMailNotification::executeAction(Device *device, const Action &action)
{
    if(action.actionTypeId() == sendMailActionTypeId) {
        SmtpClient *smtpClient = m_clients.key(device);
        smtpClient->sendMail(action.param("subject").value().toString(), action.param("body").value().toString(), action.id());
        return DeviceManager::DeviceErrorAsync;
    }
    return DeviceManager::DeviceErrorActionTypeNotFound;
}

void DevicePluginMailNotification::deviceRemoved(Device *device)
{
    SmtpClient *smtpClient = m_clients.key(device);
    m_clients.remove(smtpClient);
    delete smtpClient;
}

void DevicePluginMailNotification::testLoginFinished(const bool &success)
{
    SmtpClient *smtpClient = static_cast<SmtpClient*>(sender());
    Device *device = m_clients.value(smtpClient);
    if(success) {
        emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusSuccess);
    } else {
        emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusFailure);
        if(m_clients.contains(smtpClient)) {
            m_clients.remove(smtpClient);
        }
        smtpClient->deleteLater();
    }
}

void DevicePluginMailNotification::sendMailFinished(const bool &success, const ActionId &actionId)
{
    if(success) {
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorNoError);
    } else {
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorDeviceNotFound);
    }
}
