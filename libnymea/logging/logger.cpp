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

#include "logger.h"
#include "logengine.h"
#include <QLoggingCategory>

Logger::Logger(LogEngine *engine, const QString &name, const QStringList &tagNames, Types::LoggingType loggingType):
    m_engine(engine),
    m_name(name),
    m_tagNames(tagNames),
    m_loggingType(loggingType)
{

}

QString Logger::name() const
{
    return m_name;
}

QStringList Logger::tagNames() const
{
    return m_tagNames;
}

Types::LoggingType Logger::loggingType() const
{
    return m_loggingType;
}

void Logger::log(const QStringList &tags, const QVariantMap &values)
{
    m_engine->logEvent(this, tags, values);
}
