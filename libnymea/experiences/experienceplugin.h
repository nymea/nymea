#ifndef EXPERIENCEPLUGIN_H
#define EXPERIENCEPLUGIN_H

#include <QObject>

class ExperiencePlugin : public JsonHandler
{
    Q_OBJECT
public:
    explicit ExperiencePlugin(QObject *parent = nullptr);

signals:

public slots:
};

Q_DECLARE_INTERFACE(ExperiencePlugin, "io.nymea.ExperiencePlugin")


#endif // EXPERIENCEPLUGIN_H
