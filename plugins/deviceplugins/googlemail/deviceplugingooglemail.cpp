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

    QVariantList actionParamsMail;
    QVariantMap actionParamSubject;
    actionParamSubject.insert("name", "subject");
    actionParamSubject.insert("type", "string");
    actionParamsMail.append(actionParamSubject);

    QVariantMap actionParamBody;
    actionParamBody.insert("name", "body");
    actionParamBody.insert("type", "string");
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

DeviceManager::DeviceError DevicePluginGoogleMail::executeAction(Device *device, const Action &action)
{
    qDebug() << "execute action " << sendMailActionTypeId.toString();
    if(action.actionTypeId() == sendMailActionTypeId){
        if(!m_smtpClient->login(device->paramValue("user").toString(), device->paramValue("password").toString())){
            return DeviceManager::DeviceErrorDeviceParameterError;
        }

        m_smtpClient->sendMail(device->paramValue("user").toString(), device->paramValue("sendTo").toString(), action.param("subject").value().toString(), action.param("body").value().toString());
        m_smtpClient->logout();
    }

    return DeviceManager::DeviceErrorNoError;
}

QString DevicePluginGoogleMail::pluginName() const
{
    return "Google Mail";
}

PluginId DevicePluginGoogleMail::pluginId() const
{
    return PluginId("edfbbf03-e243-4b33-bc46-b31e973febdd");
}

