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

#ifndef JSONTYPES_H
#define JSONTYPES_H

#include "deviceclass.h"
#include "event.h"
#include "action.h"
#include "actiontype.h"
#include "rule.h"

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
    DECLARE_TYPE(ruleTypes, "RuleType")
    DECLARE_OBJECT(paramType, "ParamType")
    DECLARE_OBJECT(param, "Param")
    DECLARE_OBJECT(stateType, "StateType")
    DECLARE_OBJECT(state, "State")
    DECLARE_OBJECT(eventType, "EventType")
    DECLARE_OBJECT(event, "Event")
    DECLARE_OBJECT(actionType, "ActionType")
    DECLARE_OBJECT(action, "Action")
    DECLARE_OBJECT(plugin, "Plugin")
    DECLARE_OBJECT(deviceClass, "DeviceClass")
    DECLARE_OBJECT(device, "Device")
    DECLARE_OBJECT(rule, "Rule")

    static QVariantMap packEventType(const EventType &eventType);
    static QVariantMap packEvent(const Event &event);
    static QVariantMap packActionType(const ActionType &actionType);
    static QVariantMap packAction(const Action &action);
    static QVariantMap packDeviceClass(const DeviceClass &deviceClass);
    static QVariantMap packPlugin(DevicePlugin *plugin);
    static QVariantMap packDevice(Device *device);
    static QVariantMap packRule(const Rule &rule);

    static bool validateMap(const QVariantMap &templateMap, const QVariantMap &map);
    static bool validateProperty(const QVariant &templateValue, const QVariant &value);
    static bool validateList(const QVariantList &templateList, const QVariantList &list);
    static bool validateVariant(const QVariant &templateVariant, const QVariant &variant);
    static bool validateBasicType(const QVariant &variant);
    static bool validateRuleType(const QVariant &variant);

private:
    static bool s_initialized;
    static void init();
};

#endif // JSONTYPES_H
