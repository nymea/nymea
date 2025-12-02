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

#ifndef JSONREPLY_H
#define JSONREPLY_H

#include <QObject>
#include <QVariantMap>
#include <QUuid>
#include <QTimer>

class JsonHandler;

class JsonReply: public QObject
{
    Q_OBJECT
public:
    enum Type {
        TypeSync,
        TypeAsync
    };

    static JsonReply *createReply(JsonHandler *handler, const QVariantMap &data);
    static JsonReply *createAsyncReply(JsonHandler *handler, const QString &method);

    Type type() const;
    QVariantMap data() const;
    void setData(const QVariantMap &data);

    JsonHandler *handler() const;
    QString method() const;

    QUuid clientId() const;
    void setClientId(const QUuid &clientId);

    int commandId() const;
    void setCommandId(int commandId);

    bool timedOut() const;

public slots:
    void startWait();

signals:
    void finished();

private slots:
    void timeout();

private:
    JsonReply(Type type, JsonHandler *handler, const QString &method, const QVariantMap &data = QVariantMap());
    Type m_type;
    QVariantMap m_data;

    JsonHandler *m_handler;
    QString m_method;
    QUuid m_clientId;
    int m_commandId;
    bool m_timedOut;

    QTimer m_timeout;

};

#endif // JSONREPLY_H
