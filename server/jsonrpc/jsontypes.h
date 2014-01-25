#ifndef JSONTYPES_H
#define JSONTYPES_H

#include "deviceclass.h"
#include "trigger.h"
#include "action.h"
#include "actiontype.h"

#include <QObject>

#include <QVariantMap>

class DevicePlugin;
class Device;

namespace JsonTypes
{
    QVariantMap allTypes();

    QString basicTypesRef();
    QVariantList basicTypes();

    QString paramTypeRef();
    QVariantMap paramTypeDescription();

    QString paramRef();
    QVariantMap paramDescription();

    QString stateTypeRef();
    QVariantMap stateTypeDescription();

    QString stateRef();
    QVariantMap stateDescription();

    QString triggerTypeRef();
    QVariantMap triggerTypeDescription();
    QVariantMap packTriggerType(const TriggerType &triggerType);

    QString triggerRef();
    QVariantMap triggerDescription();
    QVariantMap packTrigger(const Trigger &trigger);

    QString actionTypeRef();
    QVariantMap actionTypeDescription();
    QVariantMap packActionType(const ActionType &actionType);

    QString actionRef();
    QVariantMap actionDescription();
    QVariantMap packAction(const Action &action);

    QString deviceClassRef();
    QVariantMap deviceClassDescription();
    QVariantMap packDeviceClass(const DeviceClass &deviceClass);

    QString pluginTypeRef();
    QVariantMap pluginTypeDescription();
    QVariantMap packPlugin(DevicePlugin *plugin);

    QString deviceRef();
    QVariantMap deviceDescription();
    QVariantMap packDevice(Device *device);

    QString ruleRef();
    QVariantMap ruleDescription();
//    QVariantMap packRule(const Rule &rule);

    bool validateMap(const QVariantMap &templateMap, const QVariantMap &map);
    bool validateProperty(const QVariant &templateValue, const QVariant &value);
    bool validateList(const QVariantList &templateList, const QVariantList &list);
    bool validateVariant(const QVariant &templateVariant, const QVariant &variant);
    bool validateBasicType(const QVariant &variant);
}

#endif // JSONTYPES_H
