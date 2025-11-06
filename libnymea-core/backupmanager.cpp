/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2025, nymea GmbH
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
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "backupmanager.h"
#include "loggingcategories.h"

#include <QDir>
#include <QDateTime>
#include <QFileInfo>
#include <QProcess>

NYMEA_LOGGING_CATEGORY(dcBackup, "Backup")

BackupManager::BackupManager(QObject *parent)
    : QObject{parent}
{

}

bool BackupManager::createBackup(const QString &sourceDir, const QString &destinationDir, int maxBackups, const QString &archivePrefix)
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

    const QString timestamp = QDateTime::currentDateTimeUtc().toString("yyyyMMdd-HHmmss");
    const QString archiveBaseName = QString("%1_%2.tar.gz").arg(archivePrefix, timestamp);
    const QString archivePath = QDir(destinationDir).filePath(archiveBaseName);

    QString absSrc = QDir(sourceDir).absolutePath();
    QStringList args;
    args << "-czf" << archivePath << "-C" << absSrc << ".";

    int exitCode = QProcess::execute("tar", args);
    if (exitCode != 0) {
        qCWarning(dcBackup()) << "Creating tar failed with exit code" << exitCode << "command: tar" << args.join(' ');
        QFile::remove(archivePath);
        return false;
    }

    if (!QFileInfo::exists(archivePath)) {
        qCWarning(dcBackup()) << "Archive has not been created. Unknown error.";
        return false;
    }

    // 4) Prune old backups (keep newest 'maxBackups')
    if (maxBackups > 0) {
        const QString pattern = QString("%1_*.tar.gz").arg(archivePrefix);
        QFileInfoList files = dst.entryInfoList({pattern}, QDir::Files | QDir::NoSymLinks, QDir::Time); // sorted by time (newest first)
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

void BackupManager::restoreBackup(const QString &fileName)
{
    Q_UNUSED(fileName)
}
