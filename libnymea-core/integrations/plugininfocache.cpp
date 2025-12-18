// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "plugininfocache.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>

#include "loggingcategories.h"
#include "nymeasettings.h"

PluginInfoCache::PluginInfoCache() {}

void PluginInfoCache::cachePluginInfo(const QJsonObject &metaData)
{
    QString fileName = metaData.value("id").toString().remove(QRegularExpression("[{}]")) + ".cache";
    QDir path = NymeaSettings::cachePath() + "/plugininfo/";
    if (!path.exists()) {
        if (!path.mkpath(path.absolutePath())) {
            qCWarning(dcThingManager()) << "Error creating thing class cache dir at" << path.absolutePath();
        }
    }
    QFile file(path.absoluteFilePath(fileName));
    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        qCWarning(dcThingManager()) << "Error opening thing class cache for writing at" << path.absoluteFilePath(fileName);
        return;
    }

    file.write(QJsonDocument::fromVariant(metaData.toVariantMap()).toJson(QJsonDocument::Compact));
    file.close();
}

QJsonObject PluginInfoCache::loadPluginInfo(const PluginId &pluginId)
{
    QString fileName = pluginId.toString().remove(QRegularExpression("[{}]")) + ".cache";
    QDir path = NymeaSettings::cachePath() + "/plugininfo/";
    QFile file(path.absoluteFilePath(fileName));
    if (!file.open(QFile::ReadOnly)) {
        return QJsonObject();
    }

    QByteArray data = file.readAll();
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dcThingManager()) << "Error parsing plugin info cache entry:" << path.absoluteFilePath(fileName);
        return QJsonObject();
    }
    return QJsonObject::fromVariantMap(jsonDoc.toVariant().toMap());
}
