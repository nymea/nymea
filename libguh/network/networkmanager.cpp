/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "networkmanager.h"

NetworkManager::NetworkManager(QObject *parent) :
    QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);

    connect(m_manager, &QNetworkAccessManager::finished, this, &NetworkManager::replyFinished);
}

QNetworkReply *NetworkManager::get(const PluginId &pluginId, const QNetworkRequest &request)
{
    QNetworkReply  *reply = m_manager->get(request);
    m_replies.insert(reply, pluginId);
    return reply;
}

QNetworkReply *NetworkManager::post(const PluginId &pluginId, const QNetworkRequest &request, const QByteArray &data)
{
    QNetworkReply  *reply = m_manager->post(request, data);
    m_replies.insert(reply, pluginId);
    return reply;
}

QNetworkReply *NetworkManager::put(const PluginId &pluginId, const QNetworkRequest &request, const QByteArray &data)
{
    QNetworkReply  *reply = m_manager->put(request, data);
    m_replies.insert(reply, pluginId);
    return reply;
}

void NetworkManager::replyFinished(QNetworkReply *reply)
{
    if (reply->error()) {
        qWarning() << "ERROR: network manager reply error: " << reply->errorString();
    }

    // NOTE: Each plugin has to delete his own replys with deleteLater()!!
    PluginId pluginId = m_replies.take(reply);
    emit replyReady(pluginId, reply);
}
