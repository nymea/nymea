// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef PYTHONINTEGRATIONPLUGIN_H
#define PYTHONINTEGRATIONPLUGIN_H

#include "integrations/integrationplugin.h"

#include <QFuture>
#include <QJsonObject>
#include <QObject>
#include <QThreadPool>

extern "C" {
typedef struct _object PyObject;
typedef struct _ts PyThreadState;
typedef struct _thing PyThing;
typedef struct _is PyInterpreterState;
}

class PythonIntegrationPlugin : public IntegrationPlugin
{
    Q_OBJECT
public:
    explicit PythonIntegrationPlugin(QObject *parent = nullptr);
    ~PythonIntegrationPlugin();

    static void initPython();
    static void deinitPython();

    bool loadScript(const QString &scriptFile);

    void init() override;
    void startMonitoringAutoThings() override;
    void discoverThings(ThingDiscoveryInfo *info) override;
    void startPairing(ThingPairingInfo *info) override;
    void confirmPairing(ThingPairingInfo *info, const QString &username, const QString &secret) override;
    void setupThing(ThingSetupInfo *info) override;
    void postSetupThing(Thing *thing) override;
    void executeAction(ThingActionInfo *info) override;
    void thingRemoved(Thing *thing) override;
    void browseThing(BrowseResult *result) override;
    void executeBrowserItem(BrowserActionInfo *info) override;
    void browserItem(BrowserItemResult *result) override;

    static PyObject *pyConfiguration(PyObject *self, PyObject *args);
    static PyObject *pyConfigValue(PyObject *self, PyObject *args);
    static PyObject *pySetConfigValue(PyObject *self, PyObject *args);
    static PyObject *pyMyThings(PyObject *self, PyObject *args);
    static PyObject *pyAutoThingsAppeared(PyObject *self, PyObject *args);
    static PyObject *pyAutoThingDisappeared(PyObject *self, PyObject *args);
    static PyObject *pyPluginStorage(PyObject *self, PyObject *args);
    static PyObject *pyApiKeyStorage(PyObject *self, PyObject *args);
    static PyObject *pyHardwareManager(PyObject *self, PyObject *args);

private:
    void exportIds();
    void exportThingClass(const ThingClass &thingClass);
    void exportParamTypes(const ParamTypes &paramTypes, const QString &thingClassName, const QString &typeClass, const QString &typeName);
    void exportStateTypes(const StateTypes &stateTypes, const QString &thingClassName);
    void exportEventTypes(const EventTypes &eventTypes, const QString &thingClassName);
    void exportActionTypes(const ActionTypes &actionTypes, const QString &thingClassName);
    void exportBrowserItemActionTypes(const ActionTypes &actionTypes, const QString &thingClassName);

    bool callPluginFunction(const QString &function, PyObject *param1 = nullptr, PyObject *param2 = nullptr, PyObject *param3 = nullptr);

private:
    // The main thread state in which we create an interpreter per plugin
    static PyThreadState *s_mainThreadState;

    // A per plugin thread state and interpreter
    PyThreadState *m_threadState = nullptr;

    // A per plugin thread pool
    QThreadPool *m_threadPool = nullptr;

    // Running concurrent tasks in this plugins thread pool
    QHash<QFutureWatcher<void> *, QString> m_runningTasks;

    // The nymea module we import into the interpreter
    PyObject *m_nymeaModule = nullptr;
    // The imported plugin module (the plugin.py)
    PyObject *m_pluginModule = nullptr;
    PyObject *m_stdOutHandler = nullptr;
    PyObject *m_stdErrHandler = nullptr;

    // A map of plugin instances to plugin python scripts/modules
    // Make sure to hold the GIL when accessing this.
    static QHash<PythonIntegrationPlugin *, PyObject *> s_plugins;

    // Used for guarding access from the python threads to the plugin instance
    QMutex m_mutex;

    // Things held by this plugin instance
    QHash<Thing *, PyThing *> m_things;

    // Need to keep a copy of plugin params and sync that in a thread-safe manner
    ParamList m_pluginConfigCopy;
};

#endif // PYTHONINTEGRATIONPLUGIN_H
