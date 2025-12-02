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

/*!
    \class NetworkAccessManager
    \brief Allows to send network requests and receive replies.

    \ingroup hardware
    \inmodule libnymea

    The network manager class is a reimplementation of the \l{http://doc.qt.io/qt-5/qnetworkaccessmanager.html}{QNetworkAccessManager}
    and allows plugins to send network requests and receive replies.

    \chapter Example

    In order to make a GET request in your plugin, you can take a look at following example:

    \tt devicepluginexample.h

    \code
    #include "network/networkaccessmanager.h"

    class DevicePluginExample : public DevicePlugin
    {
    ...

    private:
        void getServerData();

    private slots:
        void onGetRequestFinished();

    ...

    };

    \endcode

    \tt devicepluginexample.cpp

    \code
        void DevicePluginExample::getServerData() {
            QNetworkReply *reply = hardwareManager()->networkManager()->get(QNetworkRequest(QUrl("http://example.com")));
            connect(reply, &QNetworkReply::finished, this, &DevicePluginExample::onGetRequestFinished);
        }

        void DevicePluginExample::onGetRequestFinished() {
            QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
            int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            if (httpStatus != 200 || reply->error() != QNetworkReply::NoError) {
                qCWarning(dcExample()) << "Get data reply error: " << httpStatus << reply->errorString();
                reply->deleteLater();
                return;
            }

            QByteArray data = reply->readAll();
            reply->deleteLater();

            ...

        }
    \endcode

    \sa HardwareResource, HardwareManager::networkManager()
*/

/*! \fn NetworkAccessManager::~NetworkAccessManager();
    Destroys this NetworkAccessManager.
*/

/*! \fn QNetworkReply *NetworkAccessManager::get(const QNetworkRequest &request);
    Posts a \a request to obtain the contents of the target request and returns a new QNetworkReply object opened for reading which emits the readyRead() signal whenever new data arrives.
    The contents as well as associated headers will be downloaded.

    \sa post(), put(), deleteResource(), sendCustomRequest()
*/

/*! \fn QNetworkReply *NetworkAccessManager::deleteResource(const QNetworkRequest &request);
    Sends a \a request to delete the resource identified by the URL of request.

    \note This feature is currently available for HTTP only, performing an HTTP DELETE request.
    \sa post(), put(), deleteResource(), sendCustomRequest()
*/

/*! \fn QNetworkReply *NetworkAccessManager::head(const QNetworkRequest &request);
    Posts a \a request to obtain the network headers for request and returns a new QNetworkReply object which will contain such headers.

    The function is named after the HTTP request associated (HEAD).
    \sa post(), put(), deleteResource(), sendCustomRequest()
*/

/*! \fn QNetworkReply *NetworkAccessManager::post(const QNetworkRequest &request, QIODevice *data);
    Sends an HTTP POST \a request to the destination specified by request and returns a new QNetworkReply object opened for reading that will contain the reply sent by the server. The contents of the \a data device will be uploaded to the server.
    Data must be open for reading and must remain valid until the finished() signal is emitted for this reply.

    \note Sending a POST request on protocols other than HTTP and HTTPS is undefined and will probably fail.

    \sa get(), put(), deleteResource(), sendCustomRequest()
*/

/*! \fn QNetworkReply *NetworkAccessManager::post(const QNetworkRequest &request, const QByteArray &data);
    This is an overloaded function.
    Sends the contents of the \a data byte array to the destination specified by \a request.
*/

/*! \fn QNetworkReply *NetworkAccessManager::post(const QNetworkRequest &request, QHttpMultiPart *multiPart);
    This is an overloaded function.
    Sends the contents of the \a multiPart message to the destination specified by \a request.
    This can be used for sending MIME multipart messages over HTTP.
*/

/*! \fn QNetworkReply *NetworkAccessManager::put(const QNetworkRequest &request, QIODevice *data);
    Uploads the contents of \a data to the destination \a request and returnes a new QNetworkReply object that will be open for reply.
    data must be opened for reading when this function is called and must remain valid until the finished() signal is emitted for this reply.
    Whether anything will be available for reading from the returned object is protocol dependent. For HTTP, the server may send a small HTML page indicating the upload was successful (or not).
    Other protocols will probably have content in their replies.

    \note For HTTP, this request will send a PUT request, which most servers do not allow. Form upload mechanisms, including that of uploading files through HTML forms, use the POST mechanism.

    \sa get(), post(), deleteResource(), sendCustomRequest()
*/

/*! \fn QNetworkReply *NetworkAccessManager::put(const QNetworkRequest &request, const QByteArray &data);
    This is an overloaded function.
    Sends the contents of the \a data byte array to the destination specified by \a request.
*/

/*! \fn QNetworkReply *NetworkAccessManager::put(const QNetworkRequest &request, QHttpMultiPart *multiPart);
    This is an overloaded function.
    Sends the contents of the \a multiPart message to the destination specified by \a request.
    This can be used for sending MIME multipart messages over HTTP.
*/

/*! \fn QNetworkReply *NetworkAccessManager::sendCustomRequest(const QNetworkRequest &request, const QByteArray &verb, QIODevice *data = nullptr);
    Sends a custom \a request to the server identified by the URL of request.
    It is the user's responsibility to send a \a verb to the server that is valid according to the HTTP specification.
    This method provides means to send verbs other than the common ones provided via get() or post() etc., for instance sending an HTTP OPTIONS command.
    If \a data is not empty, the contents of the data device will be uploaded to the server; in that case, data must be open for reading and must remain valid until the finished() signal is emitted for this reply.

    \sa get(), post(), put(), deleteResource()
*/



#include "networkaccessmanager.h"
#include "loggingcategories.h"

/*! Construct the hardware resource NetworkAccessManager with the given \a parent. */
NetworkAccessManager::NetworkAccessManager(QObject *parent) :
    HardwareResource("Network access manager" , parent)
{
}
