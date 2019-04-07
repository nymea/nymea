/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef SYSTEMHANDLER_H
#define SYSTEMHANDLER_H

#include <QObject>

#include "jsonhandler.h"

namespace nymeaserver {

class System;

class SystemHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit SystemHandler(System* system, QObject *parent = nullptr);

    QString name() const override;

    Q_INVOKABLE JsonReply *GetCapabilities(const QVariantMap &params);

    Q_INVOKABLE JsonReply *Reboot(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *Shutdown(const QVariantMap &params) const;

    Q_INVOKABLE JsonReply *GetUpdateStatus(const QVariantMap &params) const;
    Q_INVOKABLE JsonReply *StartUpdate(const QVariantMap &params);
    Q_INVOKABLE JsonReply *SelectChannel(const QVariantMap &params);

signals:
    void UpdateStatusChanged(const QVariantMap &params);

private slots:
    void onUpdateStatusChanged();
private:
    System *m_system = nullptr;
};

}

#endif // SYSTEMHANDLER_H
