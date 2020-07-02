#ifndef PYTHONINTEGRATIONPLUGIN_H
#define PYTHONINTEGRATIONPLUGIN_H

#include "integrations/integrationplugin.h"

#include <QObject>
#include <QJsonObject>
#include <QFuture>

extern "C" {
typedef struct _object PyObject;
typedef struct _ts PyThreadState;
typedef struct _thing PyThing;
}


class PythonIntegrationPlugin : public IntegrationPlugin
{
    Q_OBJECT
public:
    explicit PythonIntegrationPlugin(QObject *parent = nullptr);
    ~PythonIntegrationPlugin();

    static void initPython();

    bool loadScript(const QString &scriptFile);

    void init() override;
    void startMonitoringAutoThings() override;
    void discoverThings(ThingDiscoveryInfo *info) override;
    void setupThing(ThingSetupInfo *info) override;
    void postSetupThing(Thing *thing) override;
    void executeAction(ThingActionInfo *info) override;
    void thingRemoved(Thing *thing) override;


    static void dumpError();
    static PyObject* pyConfiguration(PyObject* self, PyObject* args);
    static PyObject* pyConfigValue(PyObject* self, PyObject* args);
    static PyObject* pySetConfigValue(PyObject* self, PyObject* args);
    static PyObject* pyMyThings(PyObject *self, PyObject* args);
    static PyObject* pyAutoThingsAppeared(PyObject *self, PyObject* args);

private:
    void exportIds();
    void exportThingClass(const ThingClass &thingClass);
    void exportParamTypes(const ParamTypes &paramTypes, const QString &thingClassName, const QString &typeClass, const QString &typeName);
    void exportStateTypes(const StateTypes &stateTypes, const QString &thingClassName);
    void exportEventTypes(const EventTypes &eventTypes, const QString &thingClassName);
    void exportActionTypes(const ActionTypes &actionTypes, const QString &thingClassName);
    void exportBrowserItemActionTypes(const ActionTypes &actionTypes, const QString &thingClassName);


    bool callPluginFunction(const QString &function, PyObject *param1 = nullptr, PyObject *param2 = nullptr);
    void cleanupPyThing(PyThing *pyThing);

private:
    static PyThreadState* s_mainThread;

    // Modules imported into the global context
    static PyObject *s_nymeaModule;
    static PyObject *s_asyncio;

    // A map of plugin instances to plugin python scripts/modules
    // Make sure to hold the GIL when accessing this.
    static QHash<PythonIntegrationPlugin*, PyObject*> s_plugins;

    // Used for guarding access from the python threads to the plugin instance
    QMutex m_mutex;

    // The imported plugin module
    PyObject *m_module = nullptr;

    // Things held by this plugin instance
    QHash<Thing*, PyThing*> m_things;

    // Need to keep a copy of plugin params and sync that in a thread-safe manner
    ParamList m_pluginConfigCopy;
};

#endif // PYTHONINTEGRATIONPLUGIN_H
