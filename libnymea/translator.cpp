#include "translator.h"
#include "nymeasettings.h"

#include "loggingcategories.h"
#include "plugin/deviceplugin.h"
#include <QCoreApplication>
#include <QDir>

Translator::Translator(DeviceManager *deviceManager):
    m_deviceManager(deviceManager)
{

}

Translator::~Translator()
{
    foreach (const TranslatorContext &ctx, m_translatorContexts) {
        foreach (QTranslator *t, ctx.translators) {
            t->deleteLater();
        }
    }
    m_translatorContexts.clear();
}

//DeviceClass Translator::translate(DevicePlugin* plugin, const DeviceClass &deviceClass, const QLocale &locale)
//{
//    if (!m_translatorContexts.contains(deviceClass.pluginId()) || !m_translatorContexts.value(deviceClass.pluginId()).translators.contains(locale)) {
//        loadTranslator(plugin, locale);
//    }

//    QTranslator* translator = m_translatorContexts.value(deviceClass.pluginId()).translators.value(locale);

//    DeviceClass translatedDeviceClass(deviceClass);
//    QString translatedDisplayName = translator->translate(plugin->pluginName().toUtf8(), deviceClass.displayName().toUtf8());
//    if (!translatedDisplayName.isEmpty()) {
//        translatedDeviceClass.setDisplayName(translatedDisplayName);
//    }

//    // ParamTypes
//    ParamTypes translatedParamTypes;
//    foreach (ParamType paramType, deviceClass.paramTypes()) {
//        QString translated = translator->translate(plugin->pluginName().toUtf8(), paramType.displayName().toUtf8());
//        if (!translated.isEmpty()) {
//            paramType.setDisplayName(translated);
//        }
//        translatedParamTypes.append(paramType);
//    }
//    translatedDeviceClass.setParamTypes(translatedParamTypes);

//    // DiscoveryParamTypes
//    ParamTypes translatedDiscoveryParamTypes;
//    foreach (ParamType discoveryParamType, deviceClass.discoveryParamTypes()) {
//        QString translated = translator->translate(plugin->pluginName().toUtf8(), discoveryParamType.displayName().toUtf8());
//        if (!translated.isEmpty()) {
//            discoveryParamType.setDisplayName(translated);
//        }
//        translatedParamTypes.append(discoveryParamType);
//    }
//    translatedDeviceClass.setDiscoveryParamTypes(translatedDiscoveryParamTypes);

//    // EventTypes
//    EventTypes translatedEventTypes;
//    foreach (EventType eventType, deviceClass.eventTypes()) {
//        QString translated = translator->translate(plugin->pluginName().toUtf8(), eventType.displayName().toUtf8());
//        if (!translated.isEmpty()) {
//            eventType.setDisplayName(translated);
//        }
//        ParamTypes translatedEventParamTypes;
//        foreach (ParamType paramType, eventType.paramTypes()) {
//            translated = translator->translate(plugin->pluginName().toUtf8(), paramType.displayName().toUtf8());
//            if (!translated.isEmpty()) {
//                paramType.setDisplayName(translated);
//            }
//            translatedEventParamTypes.append(paramType);
//        }
//        eventType.setParamTypes(translatedEventParamTypes);
//        translatedEventTypes.append(eventType);
//    }
//    translatedDeviceClass.setEventTypes(translatedEventTypes);

//    // StateTypes
//    StateTypes translatedStateTypes;
//    foreach (StateType stateType, deviceClass.stateTypes()) {
//        QString translated = translator->translate(plugin->pluginName().toUtf8(), stateType.displayName().toUtf8());
//        if (!translated.isEmpty()) {
//            stateType.setDisplayName(translated);
//        }
//        translatedStateTypes.append(stateType);
//    }
//    translatedDeviceClass.setStateTypes(translatedStateTypes);

//    // ActionTypes
//    ActionTypes translatedActionTypes;
//    foreach (ActionType actionType, deviceClass.actionTypes()) {
//        QString translated = translator->translate(plugin->pluginName().toUtf8(), actionType.displayName().toUtf8());
//        if (!translated.isEmpty()) {
//            actionType.setDisplayName(translated);
//        }
//        ParamTypes translatedEventParamTypes;
//        foreach (ParamType paramType, actionType.paramTypes()) {
//            translated = translator->translate(plugin->pluginName().toUtf8(), paramType.displayName().toUtf8());
//            if (!translated.isEmpty()) {
//                paramType.setDisplayName(translated);
//            }
//            translatedEventParamTypes.append(paramType);
//        }
//        actionType.setParamTypes(translatedEventParamTypes);
//        translatedActionTypes.append(actionType);
//    }
//    translatedDeviceClass.setActionTypes(translatedActionTypes);
//    return translatedDeviceClass;
//}

QString Translator::translate(const PluginId &pluginId, const QString &string, const QLocale &locale)
{
    DevicePlugin *plugin = m_deviceManager->plugin(pluginId);

    if (!m_translatorContexts.contains(plugin->pluginId()) || !m_translatorContexts.value(plugin->pluginId()).translators.contains(locale.name())) {
        loadTranslator(plugin, locale);
    }

    QTranslator* translator = m_translatorContexts.value(plugin->pluginId()).translators.value(locale.name());
    QString translatedString = translator->translate(plugin->pluginName().toUtf8(), string.toUtf8());
    return translatedString.isEmpty() ? string : translatedString;
}

void Translator::loadTranslator(DevicePlugin *plugin, const QLocale &locale)
{
    if (!m_translatorContexts.contains(plugin->pluginId())) {
        // Create default translator for this plugin
        TranslatorContext defaultCtx;
        defaultCtx.pluginId = plugin->pluginId();
        defaultCtx.translators.insert("en_US", new QTranslator());
        m_translatorContexts.insert(plugin->pluginId(), defaultCtx);
        if (locale == QLocale("en_US")) {
            return;
        }
    }

    bool loaded = false;
    // check if there are local translations
    QTranslator* translator = new QTranslator();

    if (plugin->isBuiltIn()) {
        if (translator->load(locale, QCoreApplication::instance()->applicationName(), "-", QDir(QCoreApplication::applicationDirPath() + "../../translations/").absolutePath(), ".qm")) {
            qCDebug(dcTranslations()) << "* Loaded translation" << locale.name() << "for plugin" << plugin->pluginName() << "from" << QDir(QCoreApplication::applicationDirPath() + "../../translations/").absolutePath() + "/" + QCoreApplication::applicationName() + "-[" + locale.name() + "].qm";
            loaded = true;
        } else if (translator->load(locale, QCoreApplication::instance()->applicationName(), "-", NymeaSettings::translationsPath(), ".qm")) {
            qCDebug(dcTranslations()) << "* Loaded translation" << locale.name() << "for plugin" << plugin->pluginName() << "from" << NymeaSettings::translationsPath()+ "/" + QCoreApplication::applicationName() + "-[" + locale.name() + "].qm";
            loaded = true;
        }
    } else {
        QString pluginId = plugin->pluginId().toString().remove(QRegExp("[{}]"));

        QStringList searchDirs = QString(qgetenv("NYMEA_PLUGINS_PATH")).split(':');
        searchDirs << QCoreApplication::applicationDirPath() + "/../lib/nymea/plugins";
        searchDirs << QCoreApplication::applicationDirPath() + "/../plugins/";
        searchDirs << QCoreApplication::applicationDirPath() + "/../../../plugins/";
        searchDirs << QString("%1").arg(NYMEA_PLUGINS_PATH);

        foreach (const QString &pluginPath, searchDirs) {
            if (translator->load(locale, pluginId, "-", QDir(pluginPath + "/translations/").absolutePath(), ".qm")) {
                qCDebug(dcTranslations()) << "* Loaded translation" << locale.name() << "for plugin" << plugin->pluginName() << "from" << QDir(pluginPath + "/translations/").absolutePath();
                loaded = true;
                break;
            }
            foreach (const QString &subdir, QDir(pluginPath).entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
                if (translator->load(locale, pluginId, "-", QDir(pluginPath + "/" + subdir + "/translations/").absolutePath(), ".qm")) {
                    qCDebug(dcTranslations()) << "* Loaded translation" << locale.name() << "for plugin" << plugin->pluginName() << "from" << QDir(pluginPath + "/" + subdir + "/translations/").absolutePath() + "/" + pluginId + "-[" + locale.name() + "].qm";
                    loaded = true;
                    break;
                }
            }
            if (loaded) {
                break;
            }
        }

        // otherwise use the system translations
        if (!loaded && translator->load(locale, pluginId, "-", NymeaSettings::translationsPath(), ".qm")) {
            qCDebug(dcTranslations()) << "* Load translation" << locale.name() << "for" << plugin->pluginName() << "from" <<  NymeaSettings::translationsPath() + "/" + pluginId + "-[" + locale.name() + "].qm";
            loaded = true;
        }

        if (!loaded && locale.name() != "en_US") {
            qCWarning(dcTranslations()) << "* Could not load translation" << locale.name() << "for plugin" << plugin->pluginName() << "(" << pluginId << ")";
        }
    }


    if (!loaded) {
        translator = m_translatorContexts.value(plugin->pluginId()).translators.value("en_US");
    }

    if (!m_translatorContexts.contains(plugin->pluginId())) {
        TranslatorContext ctx;
        ctx.pluginId = plugin->pluginId();
        m_translatorContexts.insert(plugin->pluginId(), ctx);
    }
    m_translatorContexts[plugin->pluginId()].translators.insert(locale.name(), translator);

}
