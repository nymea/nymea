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

#ifndef LOGGER_H
#define LOGGER_H

#include <QStringList>
#include <QVariant>
#include "typeutils.h"

class LogEngine;

class Logger
{
public:

    QString name() const;
    QStringList tagNames() const;
    Types::LoggingType loggingType() const;

    void log(const QStringList &tags, const QVariantMap &values);

private:
    friend class LogEngine;
    Logger(LogEngine *engine, const QString &name, const QStringList &tagNames, Types::LoggingType loggingType);
    LogEngine *m_engine = nullptr;
    QString m_name;
    QStringList m_tagNames;
    Types::LoggingType m_loggingType = Types::LoggingTypeDiscrete;
};

Q_DECLARE_METATYPE(Logger*)

#endif // LOGGER_H
