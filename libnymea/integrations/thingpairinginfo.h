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

#ifndef THINGPAIRINGINFO_H
#define THINGPAIRINGINFO_H

#include <QObject>
#include <QUrl>

#include "thing.h"

class ThingManager;

class LIBNYMEA_EXPORT ThingPairingInfo: public QObject
{
    Q_OBJECT
public:
    ThingPairingInfo(const PairingTransactionId &pairingTransactionId, const ThingClassId &thingClassId, const ThingId &thingId, const QString &thingName, const ParamList &params, const ThingId &parentId, ThingManager *parent, bool reconfigure, quint32 timeout = 0);

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
