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

#include "browseresult.h"

#include <QTimer>

BrowseResult::BrowseResult(Thing *thing, const QString &itemId, const QLocale &locale, QObject *parent, quint32 timeout):
    QObject(parent),
    m_thing(thing),
    m_itemId(itemId),
    m_locale(locale)
{
    connect(this, &BrowseResult::finished, this, &BrowseResult::deleteLater, Qt::QueuedConnection);

    if (timeout > 0) {
        QTimer::singleShot(timeout, this, [this] {
            emit aborted();
            finish(Thing::ThingErrorTimeout);
        });
    }
}

Thing *BrowseResult::thing() const
{
    return m_thing;
}

QString BrowseResult::itemId() const
{
    return m_itemId;
}

QLocale BrowseResult::locale() const
{
    return m_locale;
}

BrowserItems BrowseResult::items() const
{
    return m_items;
}

bool BrowseResult::isFinished() const
{
    return m_finished;
}

Thing::ThingError BrowseResult::status() const
{
    return m_status;
}

void BrowseResult::addItem(const BrowserItem &item)
{
    m_items.append(item);
}

void BrowseResult::addItems(const BrowserItems &items)
{
    m_items.append(items);
}

void BrowseResult::finish(Thing::ThingError status)
{
    m_finished = true;
    m_status = status;
    staticMetaObject.invokeMethod(this, "finished", Qt::QueuedConnection);
}
