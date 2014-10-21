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

#include <QDebug>
#include <QJsonDocument>
#include <QVariantMap>
#include <QDateTime>

DeviceClassId googleMailDeviceClassId = DeviceClassId("3869884a-1592-4b8f-84a7-994be18ff555");
DeviceClassId customMailDeviceClassId = DeviceClassId("f4844c97-7ca6-4349-904e-ff9749a9fe74");

ActionTypeId sendMailActionTypeId = ActionTypeId("054613b0-3666-4dad-9252-e0ebca187edc");

DevicePluginMailNotification::DevicePluginMailNotification()
{
}

DevicePluginMailNotification::~DevicePluginMailNotification()
{
}

DeviceManager::DeviceSetupStatus DevicePluginMailNotification::setupDevice(Device *device)
{
    // Google mail
    if(device->deviceClassId() == googleMailDeviceClassId){
        device->setName("Google Mail (" + device->paramValue("user").toString() + ")");
        SmtpClient *smtpClient = new SmtpClient("smtp.gmail.com",
                                                465,
                                                device->paramValue("user").toString(),
                                                device->paramValue("password").toString(),
                                                SmtpClient::AuthLogin,
                                                SmtpClient::EncryptionSSL,
                                                this);
        smtpClient->setSender(device->paramValue("user").toString());
        smtpClient->setRecipiant(device->paramValue("recipient").toString());

        connect(smtpClient, &SmtpClient::sendMailFinished, this, &DevicePluginMailNotification::sendMailFinished);

        // TODO: test connection;
        m_clients.insert(smtpClient,device);
        return DeviceManager::DeviceSetupStatusSuccess;
        //return DeviceManager::DeviceSetupStatusAsync;
    }
    // Custom mail
    if(device->deviceClassId() == customMailDeviceClassId){
        device->setName("Custom Mail (" + device->paramValue("sender mail").toString() + ")");
        SmtpClient *smtpClient = new SmtpClient(this);
        smtpClient->setHost(device->paramValue("SMTP server").toString());
        smtpClient->setPort(device->paramValue("port").toInt());
        smtpClient->setUser(device->paramValue("user").toString());
        smtpClient->setPassword(device->paramValue("password").toString());

        if(device->paramValue("authentification").toString() == "PLAIN"){
            smtpClient->setAuthMethod(SmtpClient::AuthPlain);
        }
        if(device->paramValue("authentification").toString() == "LOGIN"){
            smtpClient->setAuthMethod(SmtpClient::AuthLogin);
        }

        if(device->paramValue("encryption").toString() == "NONE"){
            smtpClient->setEncryptionType(SmtpClient::EncryptionNone);
        }
        if(device->paramValue("encryption").toString() == "SSL"){
            smtpClient->setEncryptionType(SmtpClient::EncryptionSSL);
        }
        if(device->paramValue("encryption").toString() == "TLS"){
            smtpClient->setEncryptionType(SmtpClient::EncryptionTLS);
        }
        smtpClient->setRecipiant(device->paramValue("recipient").toString());
        smtpClient->setSender(device->paramValue("sender mail").toString());

        connect(smtpClient, &SmtpClient::sendMailFinished, this, &DevicePluginMailNotification::sendMailFinished);

        // TODO: test connection;
        m_clients.insert(smtpClient,device);
        return DeviceManager::DeviceSetupStatusSuccess;
        //return DeviceManager::DeviceSetupStatusAsync;
    }
    return DeviceManager::DeviceSetupStatusFailure;
}

DeviceManager::HardwareResources DevicePluginMailNotification::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

DeviceManager::DeviceError DevicePluginMailNotification::executeAction(Device *device, const Action &action)
{
    if(action.actionTypeId() == sendMailActionTypeId){
        SmtpClient *smtpClient= m_clients.key(device);
        smtpClient->sendMail(action.param("subject").value().toString(), action.param("body").value().toString(), action.id());

        return DeviceManager::DeviceErrorAsync;
    }
    return DeviceManager::DeviceErrorActionTypeNotFound;
}

void DevicePluginMailNotification::sendMailFinished(const bool &success, const ActionId &actionId)
{
    if(success){
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorNoError);
    }else{
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorDeviceNotFound);
    }
}
