/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.guru>                 *
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

#ifndef CLOUDHANDLER_H
#define CLOUDHANDLER_H

#include <QObject>

#include "guhcore.h"
#include "jsonhandler.h"
#include "loggingcategories.h"
#include "cloud/cloud.h"

namespace guhserver {

class CloudHandler : public JsonHandler
{
    Q_OBJECT
public:
    CloudHandler(QObject *parent = 0);

    QString name() const;

    Q_INVOKABLE JsonReply* Authenticate(const QVariantMap &params);
    Q_INVOKABLE JsonReply* GetConnectionStatus(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply* Enable(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply* Disable(const QVariantMap &params) const;

private:
    QList<JsonReply *> m_asyncAuthenticationReplies;

signals:
    void ConnectionStatusChanged(const QVariantMap &params);

private slots:
    void onConnectionStatusChanged();
    void onAuthenticationRequestFinished(const Cloud::CloudError &error);

};

}

#endif // CLOUDHANDLER_H
