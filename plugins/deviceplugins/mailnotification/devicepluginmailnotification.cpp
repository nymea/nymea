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

    ATTENTION: The password currently will be saved as plain text in the guh configuration file.

    \section1 Google Mail Notification
    \section2 Examples
    \section3 Adding a Google Mail Notification service
    In order to add a Google Mail Notification service you need to configure
    the "Google Mail login" (user), the password for your Gmail account and the address
    of the recipient.
    \code
    {
        "id":1,
        "method":"Devices.AddConfiguredDevice",
        "params":{
            "deviceClassId": "{38ed6ffc-f43b-48f8-aea2-8d63cdcad87e}",
            "deviceParams":{
                "user":"my.address@gmail.com",
                "password":"my_secret_password"
                "recipient":"recipient@example.com"}
            }
        }
    }
    \endcode
    Before the device will be added, the plugin trys to login. If the username or the password
    are wrong, an error message will be send.
    \code
    {
        "id": 1,
        "params": {
            "deviceId": "{0b99ea27-896a-4a23-a044-3f1441f6a9a7}",
            "errorMessage": "",
            "success": true
        },
        "status": "success"
    }
    \endcode

    \section3 Sending a mail notification
    In order to send a mail notification from a configured Gmail service use following message
    format.
    \code
    {
        "id":1,
        "method":"Actions.ExecuteAction",
        "params":{
            "actionTypeId": "{fa54f834-34d0-4aaf-b0ab-a165191d39d3}",
            "deviceId":"{0b99ea27-896a-4a23-a044-3f1441f6a9a7}",
            "params":{
                "subject":"GUH notification",
                "body":"Hello world!"
            }
        }
    }
    \endcode
    response...
    \code
    {
        "id": 1,
        "params": {
            "errorMessage": "",
            "success": true
        },
        "status": "success"
    }
    \endcode

    \section2 Plugin propertys:
        \section3 Plugin parameters
        Each configured plugin has following paramters:

        \table
            \header
                \li Name
                \li Description
                \li Data Type
            \row
                \li user
                \li This parameter holds the username (mail address) for the login
                \li string
            \row
                \li password
                \li This parameter holds the password for the login
                \li string
            \row
                \li recipient
                \li This parameter holds the mail address of the recipient of the notification
                \li string
        \endtable

        \section3 Plugin actions:
        Following list contains all plugin \l{Action}s:
            \table
            \header
                \li Name
                \li Description
                \li UUID
            \row
                \li sendMail
                \li This action sends a mail to the recipient address of the configured device
                    with a given subject and text body.
                \li fa54f834-34d0-4aaf-b0ab-a165191d39d3
            \endtable

    \section1 Custom Mail Notification
    \section2 Examples
    \section3 Adding a Custom Mail Notification service
    In order to add a Custom Mail Notification service you need to configure
    the smtp host address, the login (user), the password, the address
    of the recipient, the connection port and the authentification method (PLAIN or LOGIN).
    The plugin currently supports only SSL encryption.
    \code
    {
        "id":1,
        "method":"Devices.AddConfiguredDevice",
        "params":{
            "deviceClassId": "{38ed6ffc-f43b-48f8-aea2-8d63cdcad87e}",
            "deviceParams":{
                "user":"my.address@gmail.com",
                "password":"my_secret_password"
                "recipient":"recipient@example.com"}
                "host":"smtp.mydomain.com"}
                "port":"465"}
                "auth":"PLAIN"}
            }
        }
    }
    \endcode
    Before the device will be added, the plugin trys to login. If the username or the password
    are wrong, an error message will be send.
    \code
    {
        "id": 1,
        "params": {
            "deviceId": "{0b99ea27-896a-4a23-a044-3f1441f6a9a7}",
            "errorMessage": "",
            "success": true
        },
        "status": "success"
    }
    \endcode

    \section2 Plugin propertys:
        \section3 Plugin parameters
        Each configured plugin has following paramters:

        \table
            \header
                \li Name
                \li Description
                \li Data Type
            \row
                \li user
                \li This parameter holds the username (mail address) for the login
                \li string
            \row
                \li password
                \li This parameter holds the password for the login
                \li string
            \row
                \li recipient
                \li This parameter holds the mail address of the recipient of the notification
                \li string
            \row
                \li host
                \li This parameter holds the smtp host address from the mail server.
                \li string
            \row
                \li port
                \li This parameter holds the smtp port from the mail server.
                \li int
            \row
                \li auth
                \li This parameter holds the authentification method from the mail server.
                    Possible values are: PLAIN, LOGIN
                \li string
        \endtable

        \section3 Plugin actions:
        Following list contains all plugin \l{Action}s:
            \table
            \header
                \li Name
                \li Description
                \li UUID
            \row
                \li sendMail
                \li This action sends a mail to the recipient address of the configured device
                    with a given subject and text body.
                \li fa54f834-34d0-4aaf-b0ab-a165191d39d3
            \endtable


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
        device->setName("Google Mail (" + device->paramValue("user").toString() + ")");
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
        device->setName("Yahoo Mail (" + device->paramValue("user").toString() + ")");
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
        device->setName("Custom Mail (" + device->paramValue("sender mail").toString() + ")");
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
