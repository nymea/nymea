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

#ifndef BACKUPMANAGER_H
#define BACKUPMANAGER_H

#include <QObject>

class BackupManager : public QObject
{
    Q_OBJECT
public:
    explicit BackupManager(QObject *parent = nullptr);

    bool automaticBackupEnabled() const;
    void setAutomaticBackupEnabled(bool automaticBackupEnabled) const;

    bool createBackup(const QString &sourceDir, const QString &destinationDir, int maxBackups = 5, const QString &archivePrefix = "nymea-configuration");
    bool restoreBackup(const QString &fileName, const QString &destinationDir, bool safetyBackup = false);

private:


};

#endif // BACKUPMANAGER_H
