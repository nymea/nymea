/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2019 Michael Zanetti <michael.zanetti@nymea.io>          *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef PLATFORMUPDATECONTROLLER_H
#define PLATFORMUPDATECONTROLLER_H

#include <QObject>

class PlatformUpdateController : public QObject
{
    Q_OBJECT
public:
    explicit PlatformUpdateController(QObject *parent = nullptr);
    virtual ~PlatformUpdateController() = default;

    virtual bool updateManagementAvailable();

    virtual QString currentVersion() const;
    virtual QString candidateVersion() const;

//    virtual QMap<QString, QString> changelog() const = 0;

    virtual void checkForUpdates();
    virtual bool updateAvailable() const;
    virtual bool startUpdate();

    virtual bool rollbackAvailable() const;
    virtual bool startRollback();

    virtual bool updateInProgress() const;

    virtual QStringList availableChannels() const;
    virtual QString currentChannel() const;
    virtual bool selectChannel(const QString &channel);

signals:
    void updateStatusChanged();
};

#endif // PLATFORMUPDATECONTROLLER_H
