/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "plugininfocache.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include "nymeasettings.h"
#include "loggingcategories.h"

PluginInfoCache::PluginInfoCache()
{

}

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
