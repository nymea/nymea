#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "typeutils.h"
#include "types/deviceclass.h"

#include <QTranslator>

class DevicePlugin;
class DeviceManager;

class Translator
{
public:
    Translator(DeviceManager *deviceManager);
    ~Translator();

    QString translate(const PluginId &pluginId, const QString &string, const QLocale &locale);

private:
    void loadTranslator(DevicePlugin *plugin, const QLocale &locale);

private:
    DeviceManager *m_deviceManager = nullptr;

    struct TranslatorContext {
        PluginId pluginId;
        QHash<QLocale, QTranslator*> translators;
    };
    QHash<PluginId, TranslatorContext> m_translatorContexts;
};

#endif // TRANSLATOR_H
