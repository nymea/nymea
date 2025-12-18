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

#ifndef THINGPAIRINGINFO_H
#define THINGPAIRINGINFO_H

#include <QObject>
#include <QUrl>

#include "thing.h"

class ThingManager;

class LIBNYMEA_EXPORT ThingPairingInfo : public QObject
{
    Q_OBJECT
public:
    ThingPairingInfo(const PairingTransactionId &pairingTransactionId,
                     const ThingClassId &thingClassId,
                     const ThingId &thingId,
                     const QString &thingName,
                     const ParamList &params,
                     const ThingId &parentId,
                     ThingManager *parent,
                     bool reconfigure,
                     quint32 timeout = 0);

    PairingTransactionId transactionId() const;

    ThingClassId thingClassId() const;
    ThingId thingId() const;
    QString thingName() const;
    ParamList params() const;
    ThingId parentId() const;

    QUrl oAuthUrl() const;

    bool isReconfigure() const;

    Thing::ThingError status() const;
    QString displayMessage() const;
    QString translatedDisplayMessage(const QLocale &locale) const;

public slots:
    void setOAuthUrl(const QUrl &oAuthUrl);
    void finish(Thing::ThingError status, const QString &displayMessage = QString());

signals:
    void finished();
    void aborted();

private:
    PairingTransactionId m_transactionId;
    ThingClassId m_thingClassId;
    ThingId m_thingId;
    QString m_thingName;
    ParamList m_params;
    ThingId m_parentId;

    QUrl m_oAuthUrl;

    bool m_reconfigure = false;
    bool m_finished = false;
    Thing::ThingError m_status = Thing::ThingErrorNoError;
    QString m_displayMessage;
    ThingManager *m_thingManager = nullptr;
};

#endif // THINGPAIRINGINFO_H
