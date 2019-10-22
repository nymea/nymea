#ifndef EXPERIENCEPLUGIN_H
#define EXPERIENCEPLUGIN_H

#include <QObject>

class JsonHandler;

class ExperiencePlugin : public QObject
{
    Q_OBJECT
public:
    explicit ExperiencePlugin(QObject *parent = nullptr);

    QList<JsonHandler*> jsonHandlers() const;

protected:
    void registerJsonHandler(JsonHandler *handler);

private:
    QList<JsonHandler*> m_jsonHandlers;
};

Q_DECLARE_INTERFACE(ExperiencePlugin, "io.nymea.ExperiencePlugin")


#endif // EXPERIENCEPLUGIN_H
