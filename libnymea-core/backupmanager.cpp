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

#include "backupmanager.h"
#include "loggingcategories.h"
#include "version.h"

#include <algorithm>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QUuid>
#include <QRegularExpression>

NYMEA_LOGGING_CATEGORY(dcBackup, "Backup")

namespace {

QDateTime parseBackupTimestamp(const QString &timestampString)
{
    const QDate date = QDate::fromString(timestampString.left(8), "yyyyMMdd");
    const QTime time = QTime::fromString(timestampString.mid(8, 6), "hhmmss");
    return QDateTime(date, time, Qt::UTC);
}

bool parseBackupFileInfo(const QFileInfo &fileInfo, const QString &archivePrefix, BackupFile *backupFile)
{
    const QString escapedArchivePrefix = QRegularExpression::escape(archivePrefix);
    const QRegularExpression backupPattern(QString("^%1-(.+)-(\\d{14})(?:-([0-9a-fA-F-]+))?\\.tar\\.gz$").arg(escapedArchivePrefix));
    const QRegularExpression legacyBackupPattern(QString("^%1-(\\d{14})\\.tar\\.gz$").arg(escapedArchivePrefix));

    QString serverVersion;
    QString timestampString;

    const QRegularExpressionMatch backupMatch = backupPattern.match(fileInfo.fileName());
    if (backupMatch.hasMatch()) {
        serverVersion = backupMatch.captured(1);
        timestampString = backupMatch.captured(2);
    } else {
        const QRegularExpressionMatch legacyBackupMatch = legacyBackupPattern.match(fileInfo.fileName());
        if (!legacyBackupMatch.hasMatch()) {
            return false;
        }
        timestampString = legacyBackupMatch.captured(1);
    }

    const QDateTime timestamp = parseBackupTimestamp(timestampString);
    if (!timestamp.isValid()) {
        qCWarning(dcBackup()) << "Ignoring backup file with invalid timestamp:" << fileInfo.fileName();
        return false;
    }

    *backupFile = BackupFile(fileInfo.fileName(), serverVersion, timestamp, static_cast<double>(fileInfo.size()));
    return true;
}

}

BackupFile::BackupFile()
{
}

BackupFile::BackupFile(const QString &fileName, const QString &serverVersion, const QDateTime &timestamp, double size):
    m_fileName(fileName),
    m_serverVersion(serverVersion),
    m_timestamp(timestamp),
    m_size(size)
{
}

QString BackupFile::fileName() const
{
    return m_fileName;
}

QString BackupFile::serverVersion() const
{
    return m_serverVersion;
}

QDateTime BackupFile::timestamp() const
{
    return m_timestamp;
}

double BackupFile::size() const
{
    return m_size;
}

bool BackupFile::operator==(const BackupFile &other) const
{
    return m_fileName == other.fileName()
            && m_serverVersion == other.serverVersion()
            && m_timestamp == other.timestamp()
            && m_size == other.size();
}

bool BackupFile::operator!=(const BackupFile &other) const
{
    return !operator==(other);
}

BackupFiles::BackupFiles()
{
}

BackupFiles::BackupFiles(const QList<BackupFile> &other):
    QList<BackupFile>(other)
{
}

QVariant BackupFiles::get(int index) const
{
    return QVariant::fromValue(at(index));
}

void BackupFiles::put(const QVariant &variant)
{
    append(variant.value<BackupFile>());
}

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

BackupFiles BackupManager::backupFiles(const QString &destinationDir, const QString &archivePrefix) const
{
    BackupFiles backupFiles;

    const QDir dir(destinationDir);
    if (!dir.exists()) {
        return backupFiles;
    }

    const QString pattern = QString("%1-*.tar.gz").arg(archivePrefix);
    const QFileInfoList fileInfos = dir.entryInfoList({pattern}, QDir::Files | QDir::NoSymLinks, QDir::Name);
    for (const QFileInfo &fileInfo: fileInfos) {
        BackupFile backupFile;
        if (parseBackupFileInfo(fileInfo, archivePrefix, &backupFile)) {
            backupFiles.append(backupFile);
        }
    }

    std::sort(backupFiles.begin(), backupFiles.end(), [](const BackupFile &left, const BackupFile &right) {
        if (left.timestamp() != right.timestamp()) {
            return left.timestamp() > right.timestamp();
        }

        return left.fileName() > right.fileName();
    });

    return backupFiles;
}

bool BackupManager::createBackup(const QString &sourceDir, const QString &destinationDir, int maxBackups, const QString &archivePrefix, QString *archivePath)
{
    QFileInfo srcInfo(sourceDir);
    if (!srcInfo.exists() || !srcInfo.isDir()) {
        qCWarning(dcBackup()) << "Source directory doesn't exist or isn't a directory:" << sourceDir;
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
    const QString uniqueSuffix = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QString archiveBaseName = QString("%1-%2-%3-%4.tar.gz").arg(archivePrefix, QString::fromLatin1(NYMEA_VERSION_STRING), timestamp, uniqueSuffix);
    const QString createdArchivePath = QDir(destinationDir).filePath(archiveBaseName);

    QString absSrc = QDir(sourceDir).absolutePath();
    QStringList args;
    args << "-czf" << createdArchivePath << "-C" << absSrc << ".";

    int exitCode = QProcess::execute("tar", args);
    if (exitCode != 0) {
        qCWarning(dcBackup()) << "Creating tar failed with exit code" << exitCode << "command: tar" << args.join(' ');
        QFile::remove(createdArchivePath);
        return false;
    }

    if (!QFileInfo::exists(createdArchivePath)) {
        qCWarning(dcBackup()) << "Archive has not been created. Unknown error.";
        return false;
    }

    if (archivePath) {
        *archivePath = createdArchivePath;
    }

    if (maxBackups > 0) {
        const QString pattern = QString("%1-*.tar.gz").arg(archivePrefix);
        QFileInfoList files = dst.entryInfoList({pattern}, QDir::Files | QDir::NoSymLinks,
                                                QDir::Time); // sorted by time (newest first)
        if (files.size() <= maxBackups)
            return true;

        const QString createdArchiveAbsolutePath = QFileInfo(createdArchivePath).absoluteFilePath();
        int remainingFiles = files.size();

        // Delete the oldest backups first but never the archive we just created.
        for (int i = files.size() - 1; i >= 0 && remainingFiles > maxBackups; --i) {
            const QString path = files.at(i).absoluteFilePath();
            if (path == createdArchiveAbsolutePath) {
                continue;
            }

            if (!QFile::remove(path)) {
                qCWarning(dcBackup()) << "Warning: failed to remove old backup: " << path;
            } else {
                qCDebug(dcBackup()) << "Removed old backup: " << path;
                --remainingFiles;
            }
        }
    }

    return true;
}

bool BackupManager::restoreBackup(const QString &fileName, const QString &destinationDir, bool safetyBackup)
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
                qCWarning(dcBackup()) << "Failed to clear existing target directory:" << destinationDir;
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
        qCWarning(dcBackup()) << "tar failed with exit code" << exitCode << "during restore. Command: tar" << args.join(' ');
        return false;
    }

    qCInfo(dcBackup()) << "Resored backup" << fileName << "successfully into" << destinationDir;
    return true;
}
