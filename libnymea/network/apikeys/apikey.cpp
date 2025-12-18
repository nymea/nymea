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

#include "apikey.h"

#include "loggingcategories.h"
NYMEA_LOGGING_CATEGORY(dcApiKeys, "ApiKeys")

ApiKey::ApiKey() {}

/*!
 * \brief ApiKey::data
 * \param key
 * \return Retrns the data for key. For example data("key") or data("clientId")
 * An ApiKey can have multiple properties, like appid, clientsecret, scope information etc.
 */
QByteArray ApiKey::data(const QString &key) const
{
    return m_data.value(key);
}

/*!
 * \brief ApiKey::insert
 * Insert a key value pair in the this api key. For example insert("appid", "...").
 * An ApiKey can have multiple properties, like appid, clientsecret, scope information etc.
 * \param key
 * \param data
 */
void ApiKey::insert(const QString &key, const QByteArray &data)
{
    m_data.insert(key, data);
}
