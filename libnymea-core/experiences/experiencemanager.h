#ifndef EXPERIENCEMANAGER_H
#define EXPERIENCEMANAGER_H

#include <QObject>

namespace nymeaserver {

class JsonRPCServer;

class ExperienceManager : public QObject
{
    Q_OBJECT
public:
    explicit ExperienceManager(JsonRPCServer *jsonRpcServer, QObject *parent = nullptr);

signals:

public slots:
};

}
#endif // EXPERIENCEMANAGER_H
