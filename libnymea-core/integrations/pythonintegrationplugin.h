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

    QJsonObject metaData() const;


    void init() override;
    void discoverThings(ThingDiscoveryInfo *info) override;
    void setupThing(ThingSetupInfo *info) override;
    void postSetupThing(Thing *thing) override;
    void thingRemoved(Thing *thing) override;


    static void dumpError();
private:
    void exportIds();

    void callPluginFunction(const QString &function, PyObject *param);

private:
//    static QHash<PyObject*, PyThreadState*> s_modules;

    static PyThreadState* s_mainThread;
    static PyObject *s_nymeaModule;
    static PyObject *s_asyncio;

    QVariantMap m_metaData;
    PyObject *m_module = nullptr;
    QFuture<void> m_eventLoop;

    QHash<Thing*, PyThing*> m_things;

};

#endif // PYTHONINTEGRATIONPLUGIN_H
