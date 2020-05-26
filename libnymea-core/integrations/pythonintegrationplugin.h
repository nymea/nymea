#ifndef PYTHONINTEGRATIONPLUGIN_H
#define PYTHONINTEGRATIONPLUGIN_H

#include "integrations/integrationplugin.h"

#include <QObject>
#include <QJsonObject>

extern "C" {
typedef struct _object PyObject;
}


class PythonIntegrationPlugin : public IntegrationPlugin
{
    Q_OBJECT
public:
    explicit PythonIntegrationPlugin(QObject *parent = nullptr);

    bool loadScript(const QString &scriptFile);

    QJsonObject metaData() const;


    void init() override;
    void discoverThings(ThingDiscoveryInfo *info) override;


private:
    PyObject *m_module = nullptr;

};

#endif // PYTHONINTEGRATIONPLUGIN_H
