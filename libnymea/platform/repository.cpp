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
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "repository.h"

Repository::Repository(const QString &id, const QString &displayName, bool enabled):
    m_id(id),
    m_displayName(displayName),
    m_enabled(enabled)
{

}

Repository::Repository()
{

}

QString Repository::id() const
{
    return m_id;
}

QString Repository::displayName() const
{
    return m_displayName;
}

bool Repository::enabled() const
{
    return m_enabled;
}

void Repository::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

Repositories::Repositories()
{

}

Repositories::Repositories(const QList<Repository> &other): QList<Repository>(other)
{

}

QVariant Repositories::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void Repositories::put(const QVariant &variant)
{
    append(variant.value<Repository>());
}
