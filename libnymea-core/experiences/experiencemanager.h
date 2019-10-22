#ifndef EXPERIENCEMANAGER_H
#define EXPERIENCEMANAGER_H

#include <QObject>

class ExperiencePlugin;

namespace nymeaserver {

class JsonRPCServer;

class ExperienceManager : public QObject
{
    Q_OBJECT
public:
    explicit ExperienceManager(JsonRPCServer *jsonRpcServer, QObject *parent = nullptr);

signals:

public slots:

private slots:
    void loadPlugins();

private:
    QStringList pluginSearchDirs() const;

private:
    JsonRPCServer *m_jsonRpcServer = nullptr;

    void loadExperiencePlugin(const QString &file);

private:
    QList<ExperiencePlugin*> m_plugins;
};

}
#endif // EXPERIENCEMANAGER_H
