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

#ifndef NETWORKACCESSMANAGERIMPL_H
#define NETWORKACCESSMANAGERIMPL_H

#include "libnymea.h"
#include "typeutils.h"
#include "network/networkaccessmanager.h"

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QUrl>
#include <QTimer>

namespace nymeaserver {

class NetworkAccessManagerImpl : public NetworkAccessManager
{
    Q_OBJECT

public:
    NetworkAccessManagerImpl(QNetworkAccessManager *networkManager, QObject *parent = nullptr);

    QNetworkReply *get(const QNetworkRequest &request) override;
    QNetworkReply *deleteResource(const QNetworkRequest &request) override;
    QNetworkReply *head(const QNetworkRequest &request) override;

    QNetworkReply *post(const QNetworkRequest &request, QIODevice *data) override;
    QNetworkReply *post(const QNetworkRequest &request, const QByteArray &data) override;
    QNetworkReply *post(const QNetworkRequest &request, QHttpMultiPart *multiPart) override;

    QNetworkReply *put(const QNetworkRequest &request, QIODevice *data) override;
    QNetworkReply *put(const QNetworkRequest &request, const QByteArray &data) override;
    QNetworkReply *put(const QNetworkRequest &request, QHttpMultiPart *multiPart) override;

    QNetworkReply *sendCustomRequest(const QNetworkRequest &request, const QByteArray &verb, QIODevice *data = nullptr) override;

    bool available() const override;
    bool enabled() const override;

protected:
    void setEnabled(bool enabled) override;

private:
    bool m_available = false;
    bool m_enabled = false;

    QNetworkAccessManager *m_manager;
    QHash<QNetworkReply*, QTimer*> m_timeoutTimers;

    void hookupTimeoutTimer(QNetworkReply* reply);

private slots:
    void networkReplyFinished();
    void networkTimeout();

};

}

#endif // NETWORKACCESSMANAGER_H
