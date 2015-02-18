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

/*!
  \class NetworkManager
  \brief Allows to send network requests and receive replies.

  \ingroup hardware
  \inmodule libguh

  The network manager class is a reimplementation of the \l{http://doc-snapshot.qt-project.org/qt5-5.4/qnetworkaccessmanager.html}{QNetworkAccessManager}
  and allows plugins to send network requests and receive replies.
*/

/*!
 * \fn NetworkManager::replyReady(const PluginId &pluginId, QNetworkReply *reply)
 * This signal will be emitted whenever a pending network \a reply for the plugin with the given \a pluginId is finished.
 *
 * \sa DevicePlugin::networkManagerReplyReady()
 */

#include "networkmanager.h"

/*! Construct the hardware resource NetworkManager with the given \a parent. */
NetworkManager::NetworkManager(QObject *parent) :
    QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &NetworkManager::replyFinished);

    qDebug() << "--> Network manager created successfully.";
}

/*! Posts a request to obtain the contents of the target \a request from the plugin with the given \a pluginId
 * and returns a new QNetworkReply object opened for reading which emits the replyReady() signal whenever new
 * data arrives.
 * The contents as well as associated headers will be downloaded.
 *
 * \note The plugin has to delete the QNetworkReply with the function deleteLater().
 *
 * \sa DevicePlugin::networkManagerGet()
 */
QNetworkReply *NetworkManager::get(const PluginId &pluginId, const QNetworkRequest &request)
{
    QNetworkReply  *reply = m_manager->get(request);
    m_replies.insert(reply, pluginId);
    return reply;
}

/*! Sends an HTTP POST request to the destination specified by \a request from the plugin with the given
 * \a pluginId and returns a new QNetworkReply object opened for reading that will contain the reply sent
 * by the server. The contents of the \a data will be uploaded to the server.
 *
 * \note The plugin has to delete the QNetworkReply with the function deleteLater().
 *
 * \sa DevicePlugin::networkManagerPost()
 */
QNetworkReply *NetworkManager::post(const PluginId &pluginId, const QNetworkRequest &request, const QByteArray &data)
{
    QNetworkReply  *reply = m_manager->post(request, data);
    m_replies.insert(reply, pluginId);
    return reply;
}

/*! Uploads the contents of \a data to the destination \a request from the plugin with the given
 * \a pluginId and returnes a new QNetworkReply object that will be open for reply.
 *
 * \note The plugin has to delete the QNetworkReply with the function deleteLater().
 *
 * \sa DevicePlugin::networkManagerPut()
 */
QNetworkReply *NetworkManager::put(const PluginId &pluginId, const QNetworkRequest &request, const QByteArray &data)
{
    QNetworkReply  *reply = m_manager->put(request, data);
    m_replies.insert(reply, pluginId);
    return reply;
}

void NetworkManager::replyFinished(QNetworkReply *reply)
{
    // NOTE: Each plugin has to delete his own replys with deleteLater()!!
    // NOTE: also the reply->error() has to be handled in each plugin!!
    PluginId pluginId = m_replies.take(reply);
    emit replyReady(pluginId, reply);
}
