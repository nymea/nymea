#ifndef PYTHONINTEGRATIONPLUGIN_H
#define PYTHONINTEGRATIONPLUGIN_H

#include "integrations/integrationplugin.h"

#include <QObject>
#include <QJsonObject>

extern "C" {
typedef struct _object PyObject;
typedef struct _ts PyThreadState;
}


class PythonIntegrationPlugin : public IntegrationPlugin
{
    Q_OBJECT
public:
    explicit PythonIntegrationPlugin(QObject *parent = nullptr);

    static void initPython();

    bool loadScript(const QString &scriptFile);

    QJsonObject metaData() const;


    void init() override;
    void discoverThings(ThingDiscoveryInfo *info) override;


private:
    void dumpError();

    void exportIds();

private:
//    static QHash<PyObject*, PyThreadState*> s_modules;

    QVariantMap m_metaData;
    PyObject *m_module = nullptr;
    PyThreadState *m_interpreter = nullptr;

};

#endif // PYTHONINTEGRATIONPLUGIN_H
