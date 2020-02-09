#ifndef PLUGININFOCACHE_H
#define PLUGININFOCACHE_H

#include "types/deviceclass.h"
#include "devices/deviceplugin.h"

class PluginInfoCache
{
public:
    PluginInfoCache();

    static void cachePluginInfo(const QJsonObject &metaData);
    static QJsonObject loadPluginInfo(const PluginId &pluginId);
};

#endif // PLUGININFOCACHE_H
