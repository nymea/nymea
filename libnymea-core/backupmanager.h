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

#ifndef BACKUPMANAGER_H
#define BACKUPMANAGER_H

#include <QDateTime>
#include <QList>
#include <QMetaType>
#include <QObject>
#include <QTimer>
#include <QVariant>

class QFileSystemWatcher;
class QByteArray;
class QNetworkRequest;

class BackupFile
{
    Q_GADGET
    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QString serverVersion READ serverVersion)
    Q_PROPERTY(QDateTime timestamp READ timestamp)
    Q_PROPERTY(double size READ size)

public:
    BackupFile();
    BackupFile(const QString &fileName, const QString &serverVersion, const QDateTime &timestamp, double size);

    QString fileName() const;
    QString serverVersion() const;
    QDateTime timestamp() const;
    double size() const;

    bool operator==(const BackupFile &other) const;
    bool operator!=(const BackupFile &other) const;

private:
    QString m_fileName;
    QString m_serverVersion;
    QDateTime m_timestamp;
    double m_size = 0;
};
Q_DECLARE_METATYPE(BackupFile)

class BackupFiles : public QList<BackupFile>
{
    Q_GADGET
    Q_PROPERTY(int count READ count)

public:
    BackupFiles();
    BackupFiles(const QList<BackupFile> &other);

    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE void put(const QVariant &variant);
};
Q_DECLARE_METATYPE(BackupFiles)

class BackupManager : public QObject
{
    Q_OBJECT
public:
    explicit BackupManager(QObject *parent = nullptr);

    bool automaticBackupEnabled() const;
    void setAutomaticBackupEnabled(bool automaticBackupEnabled);
    int automaticBackupInterval() const;
    void setAutomaticBackupInterval(int automaticBackupInterval);
    void setSourceDirectory(const QString &sourceDirectory);
    void setDestinationDirectory(const QString &destinationDirectory);
    void setMaxBackups(int maxBackups);
    void setInfluxBackupEnabled(bool influxBackupEnabled);
    void setInfluxDatabaseConfiguration(const QString &host, const QString &databaseName, const QString &username, const QString &password);

    BackupFiles backupFiles(const QString &destinationDir, const QString &archivePrefix = "nymea-configuration") const;
    bool createBackup(const QString &sourceDir, const QString &destinationDir, int maxBackups = 5, const QString &archivePrefix = "nymea-configuration", QString *archivePath = nullptr);
    bool restoreBackup(const QString &fileName, const QString &destinationDir, bool safetyBackup = false);

signals:
    void automaticBackupEnabledChanged(bool automaticBackupEnabled);
    void backupFilesChanged();

private:
    enum InfluxDatabaseState {
        InfluxDatabaseStateExists,
        InfluxDatabaseStateMissing,
        InfluxDatabaseStateError
    };

    void reevaluateAutomaticBackup();
    void triggerAutomaticBackup();
    qint64 automaticBackupIntervalMs() const;
    void updateBackupDestinationDirectoryWatcher();
    void emitBackupFilesChangedIfNeeded();
    void onBackupDestinationDirectoryChanged(const QString &path);
    QString normalizedInfluxHost() const;
    QString influxBackupHostArgument() const;
    QString quotedInfluxDatabaseName() const;
    void readInfluxDatabaseConfiguration(const QString &settingsDirectoryPath);
    QNetworkRequest createInfluxQueryRequest(const QString &query) const;
    bool executeInfluxQuery(const QNetworkRequest &request, QByteArray *data) const;
    InfluxDatabaseState checkInfluxDatabaseState() const;
    bool waitForInfluxDatabaseRemoved() const;
    bool runInfluxdCommand(const QStringList &args, const QString &description) const;
    bool createInfluxBackup(const QString &destinationPath) const;
    bool dropInfluxDatabase() const;
    bool restoreInfluxBackup(const QString &backupPath) const;

    bool m_automaticBackupEnabled = false;
    int m_automaticBackupInterval = 24;
    QString m_sourceDirectory;
    QString m_destinationDirectory;
    int m_maxBackups = 5;
    bool m_influxBackupEnabled = false;
    QString m_influxHost = "127.0.0.1";
    QString m_influxDatabaseName = "nymea";
    QString m_influxUsername;
    QString m_influxPassword;
    QTimer *m_automaticBackupTimer = nullptr;
    QFileSystemWatcher *m_backupDestinationDirectoryWatcher = nullptr;
    BackupFiles m_backupFiles;
};

#endif // BACKUPMANAGER_H
