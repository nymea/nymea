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

#ifndef BROWSERACTIONINFO_H
#define BROWSERACTIONINFO_H

#include <QObject>

#include "thing.h"
#include "types/browseraction.h"

class ThingManager;

class BrowserActionInfo : public QObject
{
    Q_OBJECT
public:
    explicit BrowserActionInfo(Thing *thing, ThingManager *thingManager, const BrowserAction &browserAction, QObject *parent, quint32 timeout = 0);

    Thing *thing() const;
    BrowserAction browserAction() const;

    bool isFinished() const;
    Thing::ThingError status() const;
    QString displayMessage() const;
    QString translatedDisplayMessage(const QLocale &locale);

signals:
    void finished();
    void aborted();

public slots:
    void finish(Thing::ThingError status, const QString &displayMessage = QString());

private:
    Thing *m_thing = nullptr;
    BrowserAction m_browserAction;

    bool m_finished = false;
    Thing::ThingError m_status = Thing::ThingErrorNoError;
    QString m_displayMessage;

    ThingManager *m_thingManager = nullptr;
};

#endif // BROWSERACTIONINFO_H
