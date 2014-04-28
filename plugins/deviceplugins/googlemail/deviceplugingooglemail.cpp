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
    \page googlemail.html
    \title Google Mail Notification

    \ingroup plugins
    \ingroup services

    The Google Mail plugin allows you to send a mail notification from your Google mail
    account by performing an \l{Action}.

    ATTENTION: The password currently will be saved as plain text in the guh configuration file.

    \section1 Examples
    \section2 Adding a Google Mail Notification service
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
                "sendTo":"recipient@example.com"}
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

    \section2 Sending a mail notification
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

    \section1 Plugin propertys:
        \section2 Plugin parameters
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
                \li sendTo
                \li This parameter holds the mail address of the recipient of the notification
                \li string
        \endtable

        \section2 Plugin actions:
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

#include "deviceplugingooglemail.h"

#include "plugin/device.h"
#include "devicemanager.h"

#include <QDebug>
#include <QJsonDocument>
#include <QVariantMap>
#include <QDateTime>

extern VendorId guhVendorId;
DeviceClassId mailDeviceClassId = DeviceClassId("38ed6ffc-f43b-48f8-aea2-8d63cdcad87e");
ActionTypeId sendMailActionTypeId = ActionTypeId("fa54f834-34d0-4aaf-b0ab-a165191d39d3");

DevicePluginGoogleMail::DevicePluginGoogleMail()
{
    m_smtpClient = new SmtpClient();
}

DevicePluginGoogleMail::~DevicePluginGoogleMail()
{
    m_smtpClient->deleteLater();
}

QList<Vendor> DevicePluginGoogleMail::supportedVendors() const
{
    QList<Vendor> ret;
    Vendor mail(guhVendorId, "guh");
    ret.append(mail);
    return ret;
}

QList<DeviceClass> DevicePluginGoogleMail::supportedDevices() const
{
    QList<DeviceClass> ret;

    DeviceClass deviceClassGoogleMail(pluginId(), guhVendorId, mailDeviceClassId);
    deviceClassGoogleMail.setName("Google Mail Notification");
    deviceClassGoogleMail.setCreateMethod(DeviceClass::CreateMethodUser);

    // Params
    QList<ParamType> params;

    ParamType userParam("user", QVariant::String);
    params.append(userParam);

    ParamType passwordParam("password", QVariant::String);
    params.append(passwordParam);

    ParamType sendToParam("sendTo", QVariant::String);
    params.append(sendToParam);

    // Actions
    QList<ActionType> googleMailActions;

    QList<ParamType> actionParamsMail;
    ParamType actionParamSubject("subject", QVariant::String);
    actionParamsMail.append(actionParamSubject);

    ParamType actionParamBody("body", QVariant::String);
    actionParamsMail.append(actionParamBody);

    ActionType sendMailAction(sendMailActionTypeId);
    sendMailAction.setName("send mail");
    sendMailAction.setParameters(actionParamsMail);
    googleMailActions.append(sendMailAction);

    deviceClassGoogleMail.setActions(googleMailActions);
    deviceClassGoogleMail.setParams(params);

    ret.append(deviceClassGoogleMail);
    return ret;
}

bool DevicePluginGoogleMail::deviceCreated(Device *device)
{
    if(!m_smtpClient->login(device->paramValue("user").toString(), device->paramValue("password").toString(), SmtpClient::AuthLogin)){
        return false;
    }
    m_smtpClient->logout();
    return true;
}

DeviceManager::HardwareResources DevicePluginGoogleMail::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

QPair<DeviceManager::DeviceError, QString> DevicePluginGoogleMail::executeAction(Device *device, const Action &action)
{
    qDebug() << "execute action " << sendMailActionTypeId.toString();
    if(action.actionTypeId() == sendMailActionTypeId){
        if(!m_smtpClient->login(device->paramValue("user").toString(), device->paramValue("password").toString())){
            qDebug() << "ERROR: could nt login for sending mail";
            return report(DeviceManager::DeviceErrorDeviceParameterError, "user, password");
        }
        m_smtpClient->sendMail(device->paramValue("user").toString(), device->paramValue("sendTo").toString(), action.param("subject").value().toString(), action.param("body").value().toString());
        m_smtpClient->logout();
    }

    return report();
}

QString DevicePluginGoogleMail::pluginName() const
{
    return "Google Mail";
}

PluginId DevicePluginGoogleMail::pluginId() const
{
    return PluginId("edfbbf03-e243-4b33-bc46-b31e973febdd");
}

