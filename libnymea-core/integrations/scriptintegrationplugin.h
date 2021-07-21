/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef SCRIPTINTEGRATIONPLUGIN_H
#define SCRIPTINTEGRATIONPLUGIN_H

#ifdef WITH_QML

#include "integrations/integrationplugin.h"

#include <QQmlEngine>
#include <QJsonObject>

class ScriptThingDiscoveryInfo: public QObject
{
    Q_OBJECT
public:
    ScriptThingDiscoveryInfo(ThingDiscoveryInfo *info): QObject(info), m_info(info) {
        connect(info, &ThingDiscoveryInfo::aborted, this, &ScriptThingDiscoveryInfo::aborted);
        connect(info, &ThingDiscoveryInfo::finished, this, &ScriptThingDiscoveryInfo::finished);
    }
    Q_INVOKABLE void addThingDescriptor(const QUuid &thingClassId, const QString &title, const QString &description = QString(), const QVariantList &params = QVariantList(), const QUuid &parentId = QUuid()) {
        ParamList paramList;
        for (int i = 0; i < params.count(); i++) {
            paramList << Param(params.at(i).toMap().value("paramTypeId").toUuid(), params.at(i).toMap().value("value"));
        }
        ThingDescriptor d(thingClassId, title, description, parentId);
        d.setParams(paramList);
        m_info->addThingDescriptor(d);
    }
    Q_INVOKABLE void finish(Thing::ThingError status = Thing::ThingErrorNoError, const QString &displayMessage = QString()) {
        m_info->finish(status, displayMessage);
    }
signals:
    void aborted();
    void finished();
private:
    ThingDiscoveryInfo *m_info = nullptr;
};

class ScriptThing: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
public:
    ScriptThing(Thing *thing): QObject(thing), m_thing(thing) {
        connect(thing, &Thing::nameChanged, this, &ScriptThing::nameChanged);
    }

    QString name() const { return m_thing->name(); }
    void setName(const QString &name) { m_thing->setName(name); }

    Q_INVOKABLE QVariant paramValue(const QUuid &paramTypeId) { return m_thing->paramValue(paramTypeId); }
    Q_INVOKABLE void setParamValue(const QUuid &paramTypeId, const QVariant &value) { m_thing->setParamValue(paramTypeId, value); }

    Q_INVOKABLE QVariant stateValue(const QUuid &stateTypeId) { return m_thing->stateValue(stateTypeId); }
    Q_INVOKABLE void setStateValue(const QUuid &stateTypeId, const QVariant &value) { m_thing->setStateValue(stateTypeId, value); }

signals:
    void nameChanged();

private:
    Thing *m_thing = nullptr;
};

class ScriptThingSetupInfo: public QObject
{
    Q_OBJECT
    Q_PROPERTY(ScriptThing* thing READ thing CONSTANT)
public:
    ScriptThingSetupInfo(ThingSetupInfo *info, ScriptThing *scriptThing): QObject(info), m_info(info), m_thing(scriptThing) {
        connect(info, &ThingSetupInfo::aborted, this, &ScriptThingSetupInfo::aborted);
        connect(info, &ThingSetupInfo::finished, this, &ScriptThingSetupInfo::finished);
    }
    Q_INVOKABLE void finish(Thing::ThingError status = Thing::ThingErrorNoError, const QString &displayMessage = QString()) {
        m_info->finish(status, displayMessage);
    }
    ScriptThing* thing() const { return m_thing; }
signals:
    void aborted();
    void finished();
private:
    ThingSetupInfo *m_info = nullptr;
    ScriptThing *m_thing = nullptr;
};

class ScriptThingPairingInfo: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUuid ThingClassId READ ThingClassId CONSTANT)
    Q_PROPERTY(QUuid thingId READ thingId CONSTANT)
    Q_PROPERTY(QString thingName READ thingName CONSTANT)
    Q_PROPERTY(QUuid parentId READ parentId CONSTANT)
    Q_PROPERTY(QUrl oAuthUrl READ oAuthUrl WRITE setOAuthUrl)
public:
    ScriptThingPairingInfo(ThingPairingInfo* info): QObject(info), m_info(info) {
        connect(info, &ThingPairingInfo::aborted, this, &ScriptThingPairingInfo::aborted);
        connect(info, &ThingPairingInfo::finished, this, &ScriptThingPairingInfo::finished);
    }
    Q_INVOKABLE QVariant paramValue(const QUuid &paramTypeId) { return m_info->params().paramValue(paramTypeId); }
    Q_INVOKABLE void finish(Thing::ThingError status = Thing::ThingErrorNoError, const QString &displayMessage = QString()) {
        m_info->finish(status, displayMessage);
    }
    QUuid ThingClassId() const { return m_info->thingClassId(); }
    QUuid thingId() const { return m_info->thingId(); }
    QString thingName() const { return m_info->thingName(); }
    QUuid parentId() const { return m_info->parentId(); }
    QUrl oAuthUrl() const { return m_info->oAuthUrl(); }
    void setOAuthUrl(const QUrl &oAuthUrl) { m_info->setOAuthUrl(oAuthUrl); }
signals:
    void aborted();
    void finished();
private:
    ThingPairingInfo *m_info = nullptr;
};

class ScriptThingActionInfo: public QObject
{
    Q_OBJECT
    Q_PROPERTY(ScriptThing* thing READ thing CONSTANT)
    Q_PROPERTY(QUuid actionTypeId READ actionTypeId CONSTANT)
public:
    ScriptThingActionInfo(ThingActionInfo* info, ScriptThing* scriptThing): QObject(info), m_info(info), m_thing(scriptThing) {
        connect(info, &ThingActionInfo::finished, this, &ScriptThingActionInfo::finished);
        connect(info, &ThingActionInfo::aborted, this, &ScriptThingActionInfo::aborted);
    }
    ScriptThing* thing() const { return m_thing; }
    QUuid actionTypeId() const { return m_info->action().actionTypeId(); }
    Q_INVOKABLE QVariant paramValue(const QUuid &paramTypeId) { return m_info->action().params().paramValue(paramTypeId); }
    Q_INVOKABLE void finish(Thing::ThingError status = Thing::ThingErrorNoError, const QString &displayMessage = QString()) {
        m_info->finish(status, displayMessage);
    }

signals:
    void aborted();
    void finished();
private:
    ThingActionInfo* m_info = nullptr;
    ScriptThing* m_thing = nullptr;
};

class ScriptIntegrationPlugin : public IntegrationPlugin
{
    Q_OBJECT
public:
    explicit ScriptIntegrationPlugin(QObject *parent = nullptr);

    bool loadScript(const QString &fileName);

    void init() override;
    void startMonitoringAutoThings() override;
    void discoverThings(ThingDiscoveryInfo *info) override;
    void startPairing(ThingPairingInfo *info) override;
    void confirmPairing(ThingPairingInfo *info, const QString &username, const QString &secret) override;
    void setupThing(ThingSetupInfo *info) override;
    void postSetupThing(Thing *thing) override;
    void thingRemoved(Thing *thing) override;
    void executeAction(ThingActionInfo *info) override;

private:
    QQmlEngine *m_engine = nullptr;
    QJSValue m_pluginImport;
    QHash<Thing*, ScriptThing*> m_things;
};

#endif // WITH_QML

#endif // SCRIPTINTEGRATIONPLUGIN_H
