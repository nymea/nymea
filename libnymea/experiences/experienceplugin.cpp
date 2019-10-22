#include "experienceplugin.h"

ExperiencePlugin::ExperiencePlugin(QObject *parent) : QObject(parent)
{

}

QList<JsonHandler *> ExperiencePlugin::jsonHandlers() const
{
    return m_jsonHandlers;
}

void ExperiencePlugin::registerJsonHandler(JsonHandler *handler)
{
    m_jsonHandlers.append(handler);
}
