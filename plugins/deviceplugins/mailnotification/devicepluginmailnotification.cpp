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

extern VendorId guhVendorId;

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

QList<Vendor> DevicePluginMailNotification::supportedVendors() const
{
    QList<Vendor> ret;
    Vendor mail(guhVendorId, "guh");
    ret.append(mail);
    return ret;
}

QList<DeviceClass> DevicePluginMailNotification::supportedDevices() const
{
    QList<DeviceClass> ret;

    // General Action
    QList<ActionType> mailActions;

    QList<ParamType> actionParamsMail;
    ParamType actionParamSubject("subject", QVariant::String);
    actionParamsMail.append(actionParamSubject);

    ParamType actionParamBody("body", QVariant::String);
    actionParamsMail.append(actionParamBody);

    ActionType sendMailAction(sendMailActionTypeId);
    sendMailAction.setName("send mail");
    sendMailAction.setParameters(actionParamsMail);
    mailActions.append(sendMailAction);



    // Google Mail
    // ---------------------------------------------------------------
    DeviceClass deviceClassGoogleMail(pluginId(), guhVendorId, googleMailDeviceClassId);
    deviceClassGoogleMail.setName("Google Mail Notification");
    deviceClassGoogleMail.setCreateMethod(DeviceClass::CreateMethodUser);

    // Params
    QList<ParamType> googleMailParams;

    ParamType userGoogleParam("user", QVariant::String);
    googleMailParams.append(userGoogleParam);

    ParamType passwordGoogleParam("password", QVariant::String);
    googleMailParams.append(passwordGoogleParam);

    ParamType recipientGoogleParam("recipient", QVariant::String);
    googleMailParams.append(recipientGoogleParam);

    deviceClassGoogleMail.setActions(mailActions);
    deviceClassGoogleMail.setParamTypes(googleMailParams);

    // Custom Mail
    // ---------------------------------------------------------------
    DeviceClass deviceClassCustomMail(pluginId(), guhVendorId, customMailDeviceClassId);
    deviceClassCustomMail.setName("Custom Mail Notification");
    deviceClassCustomMail.setCreateMethod(DeviceClass::CreateMethodUser);

    // Params
    QList<ParamType> customMailParams;

    ParamType userCustomParam("user", QVariant::String);
    customMailParams.append(userCustomParam);

    ParamType passwordCustomParam("password", QVariant::String);
    customMailParams.append(passwordCustomParam);

    ParamType recipientCustomParam("recipient", QVariant::String);
    customMailParams.append(recipientCustomParam);

    ParamType hostCustomParam("host", QVariant::String);
    customMailParams.append(hostCustomParam);

    ParamType portCustomParam("port", QVariant::Int);
    customMailParams.append(portCustomParam);

    ParamType authCustomParam("auth", QVariant::String);
    customMailParams.append(authCustomParam);

    deviceClassCustomMail.setActions(mailActions);
    deviceClassCustomMail.setParamTypes(customMailParams);

    ret.append(deviceClassGoogleMail);
    ret.append(deviceClassCustomMail);
    return ret;
}

QPair<DeviceManager::DeviceSetupStatus, QString> DevicePluginMailNotification::setupDevice(Device *device)
{
    // Google mail
//    if(device->deviceClassId() == googleMailDeviceClassId){
//        m_smtpClient->setConnectionType(SmtpClient::SslConnection);
//        m_smtpClient->setAuthMethod(SmtpClient::AuthLogin);
//        m_smtpClient->setPort(465);
//        m_smtpClient->setHost("smtp.gmail.com");
//        m_smtpClient->login(device->paramValue("user").toString(), device->paramValue("password").toString());
//    }
    return reportDeviceSetup();
}

DeviceManager::HardwareResources DevicePluginMailNotification::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

QPair<DeviceManager::DeviceError, QString> DevicePluginMailNotification::executeAction(Device *device, const Action &action)
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

    return report();
}

QString DevicePluginMailNotification::pluginName() const
{
    return "Mail notification";
}

PluginId DevicePluginMailNotification::pluginId() const
{
    return PluginId("72aef158-07a3-4714-93b5-fec2f9d912d1");
}

