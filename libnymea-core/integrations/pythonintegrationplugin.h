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
    void discoverThings(ThingDiscoveryInfo *info) override;
    void setupThing(ThingSetupInfo *info) override;
    void postSetupThing(Thing *thing) override;
    void executeAction(ThingActionInfo *info) override;
    void thingRemoved(Thing *thing) override;


    static void dumpError();
private:
    void exportIds();
    void exportThingClass(const ThingClass &thingClass);
    void exportParamTypes(const ParamTypes &paramTypes, const QString &thingClassName, const QString &typeClass, const QString &typeName);
    void exportStateTypes(const StateTypes &stateTypes, const QString &thingClassName);
    void exportEventTypes(const EventTypes &eventTypes, const QString &thingClassName);
    void exportActionTypes(const ActionTypes &actionTypes, const QString &thingClassName);
    void exportBrowserItemActionTypes(const ActionTypes &actionTypes, const QString &thingClassName);


    void callPluginFunction(const QString &function, PyObject *param);

private:
//    static QHash<PyObject*, PyThreadState*> s_modules;

    static PyThreadState* s_mainThread;
    static PyObject *s_nymeaModule;
    static PyObject *s_asyncio;

    PyObject *m_module = nullptr;
    QFuture<void> m_eventLoop;

    QHash<Thing*, PyThing*> m_things;

    QStringList m_variableNames;

};

#endif // PYTHONINTEGRATIONPLUGIN_H
