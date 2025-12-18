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

#include "apikeystorage.h"

ApiKeyStorage::ApiKeyStorage(QObject *parent)
    : QObject(parent)
{}

ApiKey ApiKeyStorage::requestKey(const QString &name) const
{
    if (!m_keys.contains(name)) {
        qCWarning(dcApiKeys) << "API key not found for" << name;
    }
    return m_keys.value(name);
}

void ApiKeyStorage::insertKey(const QString &name, const ApiKey &key)
{
    if (m_keys.contains(name)) {
        m_keys[name] = key;
        emit keyUpdated(name, key);
    } else {
        m_keys.insert(name, key);
        emit keyAdded(name, key);
    }
}
