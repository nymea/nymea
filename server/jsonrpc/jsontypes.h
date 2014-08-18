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

#ifndef JSONTYPES_H
#define JSONTYPES_H

#include "plugin/deviceclass.h"
#include "plugin/devicedescriptor.h"
#include "rule.h"

#include "types/event.h"
#include "types/action.h"
#include "types/actiontype.h"
#include "types/paramtype.h"
#include "types/paramdescriptor.h"

#include <QObject>

#include <QVariantMap>
#include <QString>

class DevicePlugin;
class Device;

#define DECLARE_OBJECT(typeName, jsonName) \
    public: \
    static QString typeName##Ref() { return QStringLiteral("$ref:") + QStringLiteral(jsonName); } \
    static QVariantMap typeName##Description() { \
        if (!s_initialized) { init(); } \
        return s_##typeName; \
    } \
    private: \
    static QVariantMap s_##typeName; \
    public:

#define DECLARE_TYPE(typeName, jsonName) \
    public: \
    static QString typeName##Ref() { return QStringLiteral("$ref:") + QStringLiteral(jsonName); } \
    static QVariantList typeName() { \
        if (!s_initialized) { init(); } \
        return s_##typeName; \
    } \
    private: \
    static QVariantList s_##typeName; \
    public:

class JsonTypes
{
public:
    static QVariantMap allTypes();

    DECLARE_TYPE(basicTypes, "BasicType")
    DECLARE_TYPE(stateOperatorTypes, "StateOperatorType")
    DECLARE_TYPE(valueOperatorTypes, "ValueOperatorType")
    DECLARE_TYPE(createMethodTypes, "CreateMethodType")
    DECLARE_TYPE(setupMethodTypes, "SetupMethodType")
    DECLARE_TYPE(removePolicyTypes, "RemovePolicyType")
    DECLARE_OBJECT(paramType, "ParamType")
    DECLARE_OBJECT(param, "Param")
    DECLARE_OBJECT(paramDescriptor, "ParamDescriptor")
    DECLARE_OBJECT(stateType, "StateType")
    DECLARE_OBJECT(stateDescriptor, "StateDescriptor")
    DECLARE_OBJECT(state, "State")
    DECLARE_OBJECT(stateEvaluator, "StateEvaluator")
    DECLARE_OBJECT(eventType, "EventType")
    DECLARE_OBJECT(event, "Event")
    DECLARE_OBJECT(eventDescriptor, "EventDescriptor")
    DECLARE_OBJECT(actionType, "ActionType")
    DECLARE_OBJECT(action, "Action")
    DECLARE_OBJECT(plugin, "Plugin")
    DECLARE_OBJECT(vendor, "Vendor")
    DECLARE_OBJECT(deviceClass, "DeviceClass")
    DECLARE_OBJECT(device, "Device")
    DECLARE_OBJECT(deviceDescriptor, "DeviceDescriptor")
    DECLARE_OBJECT(rule, "Rule")

    static QVariantMap packEventType(const EventType &eventType);
    static QVariantMap packEvent(const Event &event);
    static QVariantMap packEventDescriptor(const EventDescriptor &event);
    static QVariantMap packActionType(const ActionType &actionType);
    static QVariantMap packAction(const Action &action);
    static QVariantMap packStateType(const StateType &stateType);
    static QVariantMap packStateDescriptor(const StateDescriptor &stateDescriptor);
    static QVariantMap packStateEvaluator(const StateEvaluator &stateEvaluator);
    static QVariantMap packParam(const Param &param);
    static QVariantMap packParamType(const ParamType &paramType);
    static QVariantMap packParamDescriptor(const ParamDescriptor &paramDescriptor);
    static QVariantMap packVendor(const Vendor &vendor);
    static QVariantMap packDeviceClass(const DeviceClass &deviceClass);
    static QVariantMap packPlugin(DevicePlugin *plugin);
    static QVariantMap packDevice(Device *device);
    static QVariantMap packDeviceDescriptor(const DeviceDescriptor &descriptor);
    static QVariantMap packRule(const Rule &rule);
    static QVariantList packCreateMethods(DeviceClass::CreateMethods createMethods);

    static Param unpackParam(const QVariantMap &paramMap);
    static ParamList unpackParams(const QVariantList &paramList);
    static ParamDescriptor unpackParamDescriptor(const QVariantMap &paramDescriptorMap);
    static QList<ParamDescriptor> unpackParamDescriptors(const QVariantList &paramDescriptorList);
    static EventDescriptor unpackEventDescriptor(const QVariantMap &eventDescriptorMap);

    static QPair<bool, QString> validateMap(const QVariantMap &templateMap, const QVariantMap &map);
    static QPair<bool, QString> validateProperty(const QVariant &templateValue, const QVariant &value);
    static QPair<bool, QString> validateList(const QVariantList &templateList, const QVariantList &list);
    static QPair<bool, QString> validateVariant(const QVariant &templateVariant, const QVariant &variant);
    static QPair<bool, QString> validateBasicType(const QVariant &variant);
    static QPair<bool, QString> validateStateOperatorType(const QVariant &variant);
    static QPair<bool, QString> validateCreateMethodType(const QVariant &variant);
    static QPair<bool, QString> validateSetupMethodType(const QVariant &variant);
    static QPair<bool, QString> validateValueOperatorType(const QVariant &variant);

private:
    static bool s_initialized;
    static void init();

    static QPair<bool, QString> report(bool status, const QString &message);

    static QString s_lastError;
};

#endif // JSONTYPES_H
