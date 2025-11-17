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

#ifndef THINGDISCOVERYINFO_H
#define THINGDISCOVERYINFO_H

#include <QObject>

#include "types/thingclass.h"
#include "types/param.h"
#include "thing.h"
#include "thingdescriptor.h"

class ThingManager;

class LIBNYMEA_EXPORT ThingDiscoveryInfo : public QObject
{
    Q_OBJECT
public:
    explicit ThingDiscoveryInfo(const ThingClassId &thingClassId, const ParamList &params, ThingManager *thingManager, quint32 timeout = 0);

    ThingClassId thingClassId() const;
    ParamList params() const;

    bool isFinished() const;

    Thing::ThingError status() const;

    ThingDescriptors thingDescriptors() const;

    QString displayMessage() const;
    QString translatedDisplayMessage(const QLocale &locale);

public slots:
    void addThingDescriptor(const ThingDescriptor &thingDescriptor);
    void addThingDescriptors(const ThingDescriptors &thingDescriptors);

    void finish(Thing::ThingError status, const QString &displayMessage = QString());

signals:
    void finished();
    void aborted();

private:
    ThingClassId m_thingClassId;
    ParamList m_params;

    bool m_finished = false;
    Thing::ThingError m_status;
    QString m_displayMessage;
    ThingDescriptors m_thingDescriptors;

    ThingManager *m_thingManager = nullptr;
};

#endif // THINGDISCOVERYINFO_H
