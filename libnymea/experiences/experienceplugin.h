#ifndef EXPERIENCEPLUGIN_H
#define EXPERIENCEPLUGIN_H

#include <QObject>
#include "jsonrpc/jsonhandler.h"

class ExperiencePlugin : public JsonHandler
{
    Q_OBJECT
public:
    explicit ExperiencePlugin(QObject *parent = nullptr);

signals:

public slots:
};

#endif // EXPERIENCEPLUGIN_H
