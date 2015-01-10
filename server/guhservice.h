#ifndef GUHSERVICE_H
#define GUHSERVICE_H

#include <QCoreApplication>
#include "qtservice/qtservice.h"

#include "guhcore.h"

class GuhService : public QtService<QCoreApplication>
{

public:
    explicit GuhService(int argc, char **argv);
    ~GuhService();

protected:
    void start();
};

#endif // GUHSERVICE_H
