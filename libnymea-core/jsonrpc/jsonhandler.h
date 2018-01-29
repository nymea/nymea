/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#ifndef JSONHANDLER_H
#define JSONHANDLER_H

#include "jsontypes.h"

#include <QObject>
#include <QVariantMap>
#include <QMetaMethod>
#include <QTimer>

namespace guhserver {

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

class JsonHandler : public QObject
{
    Q_OBJECT
public:
    explicit JsonHandler(QObject *parent = 0);

    virtual QString name() const = 0;

    QVariantMap introspect(QMetaMethod::MethodType);

    bool hasMethod(const QString &methodName);
    QPair<bool, QString> validateParams(const QString &methodName, const QVariantMap &params);
    QPair<bool, QString> validateReturns(const QString &methodName, const QVariantMap &returns);

signals:
    void asyncReply(int id, const QVariantMap &params);

protected:
    void setDescription(const QString &methodName, const QString &description);
    void setParams(const QString &methodName, const QVariantMap &params);
    void setReturns(const QString &methodName, const QVariantMap &returns);

    JsonReply *createReply(const QVariantMap &data) const;
    JsonReply *createAsyncReply(const QString &method) const;
    QVariantMap statusToReply(DeviceManager::DeviceError status) const;
    QVariantMap statusToReply(RuleEngine::RuleError status) const;
    QVariantMap statusToReply(Logging::LoggingError status) const;
    QVariantMap statusToReply(GuhConfiguration::ConfigurationError status) const;
    QVariantMap statusToReply(NetworkManager::NetworkManagerError status) const;

private:
    QHash<QString, QString> m_descriptions;
    QHash<QString, QVariantMap> m_params;
    QHash<QString, QVariantMap> m_returns;
};

}

#endif // JSONHANDLER_H
