// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2026, chargebyte austria GmbH
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

#ifndef BROWSERITEMRESULT_H
#define BROWSERITEMRESULT_H

#include <QLocale>
#include <QObject>

#include "thing.h"
#include "types/browseritem.h"

class ThingManager;

class BrowserItemResult : public QObject
{
    Q_OBJECT

public:
    explicit BrowserItemResult(Thing *thing, ThingManager *thingManager, const QString &itemId, const QLocale &locale, QObject *parent, quint32 timeout = 0);

    Thing *thing() const;
    QString itemId() const;
    QLocale locale() const;

    BrowserItem item() const;

    bool isFinished() const;
    Thing::ThingError status() const;
    QString displayMessage() const;
    QString translatedDisplayMessage(const QLocale &locale);

public slots:
    void finish(const BrowserItem &item);
    void finish(Thing::ThingError status, const QString &displayMessage = QString());

signals:
    void finished();
    void aborted();

private:
    Thing *m_thing = nullptr;
    QString m_itemId;
    QLocale m_locale;

    BrowserItem m_item;

    bool m_finished = false;
    Thing::ThingError m_status = Thing::ThingErrorNoError;
    QString m_displayMessage;

    ThingManager *m_thingManager = nullptr;
};

#endif // BROWSERITEMRESULT_H
