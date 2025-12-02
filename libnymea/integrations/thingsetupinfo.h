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

#ifndef THINGSETUPINFO_H
#define THINGSETUPINFO_H

#include "thing.h"

#include <QObject>

class ThingManager;

class LIBNYMEA_EXPORT ThingSetupInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Thing* thing READ thing CONSTANT)
public:
    explicit ThingSetupInfo(QObject *parent);
    explicit ThingSetupInfo(Thing *thing, ThingManager *thingManager, bool initialSetup, bool reconfigure, quint32 timeout = 0);

    Thing *thing() const;

    bool isInitialSetup() const;
    bool isReconfigure() const;
    bool isFinished() const;

    Thing::ThingError status() const;
    QString displayMessage() const;
    QString translatedDisplayMessage(const QLocale &locale);

public slots:
    void finish(Thing::ThingError status, const QString &displayMessage = QString());

signals:
    void finished();
    void aborted();

private:
    Thing *m_thing = nullptr;

    bool m_initialSetup = true;
    bool m_reconfigure = false;
    bool m_finished = false;
    Thing::ThingError m_status = Thing::ThingErrorNoError;
    QString m_displayMessage;

    ThingManager *m_thingManager = nullptr;
};

#endif // THINGSETUPINFO_H
