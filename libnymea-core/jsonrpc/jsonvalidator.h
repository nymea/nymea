/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef JSONVALIDATOR_H
#define JSONVALIDATOR_H

#include <QPair>
#include <QVariant>

namespace nymeaserver {

class JsonValidator
{
public:
    class Result {
    public:
        Result() {}
        Result(bool success, const QString &errorString = QString(), const QString &where = QString()): m_success(success), m_errorString(errorString), m_where(where) {}
        bool success() const { return m_success; }
        void setSuccess(bool success) { m_success = success; }
        QString errorString() const { return m_errorString; }
        void setErrorString(const QString &errorString) { m_errorString = errorString; }
        QString where() const { return m_where; }
        void setWhere(const QString &where) { m_where = where; }
        bool deprecated() { return m_deprecated; }
        void setDeprecated(bool deprecated) { m_deprecated = deprecated; }
    private:
        bool m_success = false;
        QString m_errorString;
        QString m_where;
        bool m_deprecated = false;
    };

    JsonValidator() {}

    static bool checkRefs(const QVariantMap &map, const QVariantMap &api);

    Result validateParams(const QVariantMap &params, const QString &method, const QVariantMap &api);
    Result validateReturns(const QVariantMap &returns, const QString &method, const QVariantMap &api);
    Result validateNotificationParams(const QVariantMap &params, const QString &notification, const QVariantMap &api);

    Result result() const;
private:
    Result validateMap(const QVariantMap &map, const QVariantMap &definition, const QVariantMap &api);
    Result validateEntry(const QVariant &value, const QVariant &definition, const QVariantMap &api);

    Result m_result;
};

}

#endif // JSONVALIDATOR_H
