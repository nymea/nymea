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

#include "backupmanager.h"
#include "loggingcategories.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QProcess>

NYMEA_LOGGING_CATEGORY(dcBackup, "Backup")

BackupManager::BackupManager(QObject *parent)
    : QObject{parent}
{}

bool BackupManager::automaticBackupEnabled() const
{
    return m_automaticBackupEnabled;
}

void BackupManager::setAutomaticBackupEnabled(bool automaticBackupEnabled)
{
    if (m_automaticBackupEnabled == automaticBackupEnabled)
        return;

    m_automaticBackupEnabled = automaticBackupEnabled;
    emit automaticBackupEnabledChanged(m_automaticBackupEnabled);
}

bool BackupManager::createBackup(const QString &sourceDir,
                                 const QString &destinationDir,
                                 int maxBackups,
                                 const QString &archivePrefix)
{
    QFileInfo srcInfo(sourceDir);
    if (!srcInfo.exists() || !srcInfo.isDir()) {
        qCWarning(dcBackup()) << "Source directory doesn't exist or isn't a directory:"
                              << sourceDir;
        return false;
    }

    QDir dst(destinationDir);
    if (!dst.exists()) {
        if (!QDir().mkpath(destinationDir)) {
            qCWarning(dcBackup()) << "Failed to create destination directory:" << destinationDir;
            return false;
        }
    }

    const QString timestamp = QDateTime::currentDateTimeUtc().toString("yyyyMMddHHmmss");
    const QString archiveBaseName = QString("%1-%2.tar.gz").arg(archivePrefix, timestamp);
    const QString archivePath = QDir(destinationDir).filePath(archiveBaseName);

    QString absSrc = QDir(sourceDir).absolutePath();
    QStringList args;
    args << "-czf" << archivePath << "-C" << absSrc << ".";

    int exitCode = QProcess::execute("tar", args);
    if (exitCode != 0) {
        qCWarning(dcBackup()) << "Creating tar failed with exit code" << exitCode << "command: tar"
                              << args.join(' ');
        QFile::remove(archivePath);
        return false;
    }

    if (!QFileInfo::exists(archivePath)) {
        qCWarning(dcBackup()) << "Archive has not been created. Unknown error.";
        return false;
    }

    if (maxBackups > 0) {
        const QString pattern = QString("%1-*.tar.gz").arg(archivePrefix);
        QFileInfoList files = dst.entryInfoList({pattern},
                                                QDir::Files | QDir::NoSymLinks,
                                                QDir::Time); // sorted by time (newest first)
        if (files.size() <= maxBackups)
            return true;

        // Delete everything after index (maxKeep-1)
        for (int i = maxBackups; i < files.size(); ++i) {
            const QString path = files.at(i).absoluteFilePath();
            if (!QFile::remove(path)) {
                qCWarning(dcBackup()) << "Warning: failed to remove old backup: " << path;
            } else {
                qCDebug(dcBackup()) << "Removed old backup: " << path;
            }
        }
    }

    return true;
}

bool BackupManager::restoreBackup(const QString &fileName,
                                  const QString &destinationDir,
                                  bool safetyBackup)
{
    QFileInfo arcInfo(fileName);
    if (!arcInfo.exists() || !arcInfo.isFile()) {
        qCWarning(dcBackup()) << "Archive not found:" << fileName;
        return false;
    }

    QFileInfo tgtInfo(destinationDir);
    if (tgtInfo.exists()) {
        if (!tgtInfo.isDir()) {
            qCWarning(dcBackup()) << "Target exists and is not a directory:" << destinationDir;
            return false;
        }

        if (safetyBackup) {
            const QString stamp = QDateTime::currentDateTimeUtc().toString("yyyyMMddHHmmss");
            const QString bakDir = destinationDir + ".bak-" + stamp;
            if (!QDir().rename(destinationDir, bakDir)) {
                qCWarning(dcBackup()) << "Failed to move existing target to backup:" << bakDir;
                return false;
            }

        } else {
            // Remove existing directory to ensure a clean restore
            QDir dir(destinationDir);
            if (!dir.removeRecursively()) {
                qCWarning(dcBackup())
                    << "Failed to clear existing target directory:" << destinationDir;
                return false;
            }
        }
    }

    if (!QDir().mkpath(destinationDir)) {
        qCWarning(dcBackup()) << "Failed to create target directory:" << destinationDir;
        return false;
    }

    const QString absTarget = QDir(destinationDir).absolutePath();
    QStringList args;
    args << "-xzf" << arcInfo.absoluteFilePath() << "-C" << absTarget;

    int exitCode = QProcess::execute("tar", args);
    if (exitCode != 0) {
        qCWarning(dcBackup()) << "tar failed with exit code" << exitCode
                              << "during restore. Command: tar" << args.join(' ');
        return false;
    }

    qCInfo(dcBackup()) << "Resored backup" << fileName << "successfully into" << destinationDir;
    return true;
}
