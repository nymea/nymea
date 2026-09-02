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

#include <QDateTime>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QScopedPointer>
#include <QTemporaryDir>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QUuid>
#include <algorithm>

#include <limits>
#include <sys/stat.h>
#include <unistd.h>

NYMEA_LOGGING_CATEGORY(dcBackup, "Backup")

namespace {

const char influxBackupMetadataDirectory[] = ".nymea-backup";
const char influxBackupDirectory[] = ".nymea-backup/influxdb";
const int influxQueryTimeoutMs = 10000;
const int influxDatabaseRemovalRetries = 10;
const int influxDatabaseRemovalRetryIntervalMs = 250;

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
        if (!legacyBackupMatch.hasMatch())
            return false;

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

bool validateBackupArchive(const QString &archivePath)
{
    const int exitCode = QProcess::execute("tar", QStringList() << "-tzf" << archivePath);
    return exitCode == 0;
}

bool copyDirectoryContents(const QString &sourcePath, const QString &destinationPath)
{
    QDir sourceDirectory(sourcePath);
    if (!sourceDirectory.exists()) {
        qCWarning(dcBackup()) << "Source directory does not exist:" << sourcePath;
        return false;
    }

    if (!QDir().mkpath(destinationPath)) {
        qCWarning(dcBackup()) << "Failed to create staging directory:" << destinationPath;
        return false;
    }

    const QFileInfoList entries = sourceDirectory.entryInfoList(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
    foreach (const QFileInfo &entry, entries) {
        const QString targetPath = QDir(destinationPath).filePath(entry.fileName());
        if (entry.isSymLink()) {
            QFile::remove(targetPath);

            const QByteArray sourcePathData = QFile::encodeName(entry.absoluteFilePath());
            struct stat linkStat;
            const int linkStatResult = lstat(sourcePathData.constData(), &linkStat);
            const int linkTargetBufferSize = linkStatResult == 0 && linkStat.st_size > 0 ? static_cast<int>(linkStat.st_size) + 1 : 4096;
            QByteArray linkTarget;
            linkTarget.resize(linkTargetBufferSize);
            const ssize_t linkTargetLength = readlink(sourcePathData.constData(), linkTarget.data(), static_cast<size_t>(linkTarget.size()));
            if (linkTargetLength < 0) {
                qCWarning(dcBackup()) << "Failed to read symbolic link target:" << entry.absoluteFilePath();
                return false;
            }
            if (linkTargetLength >= linkTarget.size()) {
                qCWarning(dcBackup()) << "Symbolic link target is too long to copy:" << entry.absoluteFilePath();
                return false;
            }

            linkTarget.truncate(static_cast<int>(linkTargetLength));
            const QByteArray targetPathData = QFile::encodeName(targetPath);
            if (symlink(linkTarget.constData(), targetPathData.constData()) != 0) {
                qCWarning(dcBackup()) << "Failed to create symbolic link in backup staging directory:" << targetPath;
                return false;
            }
        } else if (entry.isDir()) {
            if (!copyDirectoryContents(entry.absoluteFilePath(), targetPath))
                return false;
        } else {
            QFile::remove(targetPath);
            if (!QFile::copy(entry.absoluteFilePath(), targetPath)) {
                qCWarning(dcBackup()) << "Failed to copy backup source file:" << entry.absoluteFilePath() << targetPath;
                return false;
            }
            QFile::setPermissions(targetPath, entry.permissions());
        }
    }

    return true;
}

} // namespace

BackupFile::BackupFile() {}

BackupFile::BackupFile(const QString &fileName, const QString &serverVersion, const QDateTime &timestamp, double size)
    : m_fileName(fileName)
    , m_serverVersion(serverVersion)
    , m_timestamp(timestamp)
    , m_size(size)
{}

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
    return m_fileName == other.fileName() && m_serverVersion == other.serverVersion() && m_timestamp == other.timestamp() && m_size == other.size();
}

bool BackupFile::operator!=(const BackupFile &other) const
{
    return !operator==(other);
}

BackupFiles::BackupFiles() {}

BackupFiles::BackupFiles(const QList<BackupFile> &other)
    : QList<BackupFile>(other)
{}

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
{
    m_automaticBackupTimer = new QTimer(this);
    m_automaticBackupTimer->setSingleShot(true);
    connect(m_automaticBackupTimer, &QTimer::timeout, this, &BackupManager::reevaluateAutomaticBackup);

    m_backupDestinationDirectoryWatcher = new QFileSystemWatcher(this);
    connect(m_backupDestinationDirectoryWatcher, &QFileSystemWatcher::directoryChanged, this, &BackupManager::onBackupDestinationDirectoryChanged);
}

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
    reevaluateAutomaticBackup();
}

int BackupManager::automaticBackupInterval() const
{
    return m_automaticBackupInterval;
}

void BackupManager::setAutomaticBackupInterval(int automaticBackupInterval)
{
    automaticBackupInterval = qMax(1, automaticBackupInterval);
    if (m_automaticBackupInterval == automaticBackupInterval)
        return;

    m_automaticBackupInterval = automaticBackupInterval;
    reevaluateAutomaticBackup();
}

void BackupManager::setSourceDirectory(const QString &sourceDirectory)
{
    if (m_sourceDirectory == sourceDirectory)
        return;

    m_sourceDirectory = sourceDirectory;
    reevaluateAutomaticBackup();
}

void BackupManager::setDestinationDirectory(const QString &destinationDirectory)
{
    if (m_destinationDirectory == destinationDirectory)
        return;

    m_destinationDirectory = destinationDirectory;
    updateBackupDestinationDirectoryWatcher();
    emitBackupFilesChangedIfNeeded();
    reevaluateAutomaticBackup();
}

void BackupManager::setMaxBackups(int maxBackups)
{
    if (m_maxBackups == maxBackups)
        return;

    m_maxBackups = maxBackups;
    reevaluateAutomaticBackup();
}

void BackupManager::setInfluxBackupEnabled(bool influxBackupEnabled)
{
    m_influxBackupEnabled = influxBackupEnabled;
}

void BackupManager::setInfluxDatabaseConfiguration(const QString &host, const QString &databaseName, const QString &username, const QString &password)
{
    m_influxHost = host.trimmed().isEmpty() ? QString("127.0.0.1") : host;
    m_influxDatabaseName = databaseName.trimmed().isEmpty() ? QString("nymea") : databaseName;
    m_influxUsername = username;
    m_influxPassword = password;
}

QString BackupManager::normalizedInfluxHost() const
{
    if (m_influxHost.trimmed().isEmpty())
        return "127.0.0.1";

    QUrl url = QUrl::fromUserInput(m_influxHost);
    if (url.isValid() && !url.host().isEmpty())
        return url.host();

    return m_influxHost;
}

QString BackupManager::influxBackupHostArgument() const
{
    const QString host = normalizedInfluxHost();
    if (host.contains(':'))
        return host;

    return host + ":8088";
}

QString BackupManager::quotedInfluxDatabaseName() const
{
    QString databaseName = m_influxDatabaseName;
    databaseName.replace("\\", "\\\\");
    databaseName.replace("\"", "\\\"");
    return "\"" + databaseName + "\"";
}

void BackupManager::readInfluxDatabaseConfiguration(const QString &settingsDirectoryPath)
{
    QSettings settings(QDir(settingsDirectoryPath).filePath("nymead.conf"), QSettings::IniFormat);
    settings.beginGroup("Logs");
    m_influxHost = settings.value("logDBHost", m_influxHost).toString();
    m_influxDatabaseName = settings.value("logDBName", m_influxDatabaseName).toString();
    m_influxUsername = settings.value("logDBUser", m_influxUsername).toString();
    m_influxPassword = settings.value("logDBPassword", m_influxPassword).toString();
    settings.endGroup();

    if (m_influxHost.trimmed().isEmpty())
        m_influxHost = "127.0.0.1";

    if (m_influxDatabaseName.trimmed().isEmpty())
        m_influxDatabaseName = "nymea";
}

QNetworkRequest BackupManager::createInfluxQueryRequest(const QString &query) const
{
    QUrl url;
    url.setScheme("http");
    url.setHost(normalizedInfluxHost());
    url.setPort(8086);
    url.setPath("/query");

    QUrlQuery urlQuery;
    urlQuery.addQueryItem("q", query);
    url.setQuery(urlQuery);

    QNetworkRequest request(url);
    if (!m_influxUsername.isEmpty() || !m_influxPassword.isEmpty()) {
        const QByteArray auth = QByteArray(m_influxUsername.toLatin1() + ':' + m_influxPassword.toLatin1()).toBase64(QByteArray::Base64Encoding | QByteArray::KeepTrailingEquals);
        request.setRawHeader("Authorization", QString("Basic %1").arg(QString(auth)).toUtf8());
    }

    return request;
}

bool BackupManager::executeInfluxQuery(const QNetworkRequest &request, QByteArray *data) const
{
    QNetworkAccessManager networkAccessManager;
    QNetworkReply *reply = networkAccessManager.get(request);

    QEventLoop eventLoop;
    bool timedOut = false;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    QObject::connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    QObject::connect(&timeoutTimer, &QTimer::timeout, [&eventLoop, reply, &timedOut]() {
        timedOut = true;
        reply->abort();
        eventLoop.quit();
    });
    timeoutTimer.start(influxQueryTimeoutMs);
    eventLoop.exec();
    timeoutTimer.stop();

    const QByteArray responseData = reply->readAll();
    const QNetworkReply::NetworkError error = reply->error();
    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    reply->deleteLater();

    if (data)
        *data = responseData;

    if (timedOut) {
        qCWarning(dcBackup()) << "InfluxDB query timed out after" << influxQueryTimeoutMs << "ms";
        return false;
    }

    if (error != QNetworkReply::NoError || statusCode >= 400) {
        qCWarning(dcBackup()) << "InfluxDB query failed" << error << statusCode << responseData;
        return false;
    }

    return true;
}

BackupManager::InfluxDatabaseState BackupManager::checkInfluxDatabaseState() const
{
    QByteArray data;
    if (!executeInfluxQuery(createInfluxQueryRequest("SHOW DATABASES"), &data))
        return InfluxDatabaseStateError;

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        qCWarning(dcBackup()) << "Unable to parse InfluxDB database list:" << parseError.errorString() << data;
        return InfluxDatabaseStateError;
    }

    const QJsonArray results = document.object().value("results").toArray();
    foreach (const QJsonValue &resultValue, results) {
        const QJsonObject result = resultValue.toObject();
        if (result.contains("error")) {
            qCWarning(dcBackup()) << "InfluxDB database list query failed:" << result.value("error").toString();
            return InfluxDatabaseStateError;
        }

        const QJsonArray series = result.value("series").toArray();
        foreach (const QJsonValue &seriesValue, series) {
            const QJsonArray values = seriesValue.toObject().value("values").toArray();
            foreach (const QJsonValue &value, values) {
                const QJsonArray databaseValue = value.toArray();
                if (!databaseValue.isEmpty() && databaseValue.at(0).toString() == m_influxDatabaseName)
                    return InfluxDatabaseStateExists;
            }
        }
    }

    return InfluxDatabaseStateMissing;
}

bool BackupManager::waitForInfluxDatabaseRemoved() const
{
    for (int i = 0; i < influxDatabaseRemovalRetries; ++i) {
        const InfluxDatabaseState databaseState = checkInfluxDatabaseState();
        if (databaseState == InfluxDatabaseStateMissing)
            return true;

        if (databaseState == InfluxDatabaseStateError)
            return false;

        QThread::msleep(influxDatabaseRemovalRetryIntervalMs);
    }

    qCWarning(dcBackup()) << "InfluxDB database still exists after drop:" << m_influxDatabaseName;
    return false;
}

bool BackupManager::runInfluxdCommand(const QStringList &args, const QString &description) const
{
    QProcess process;
    process.start("influxd", args);
    if (!process.waitForStarted()) {
        qCWarning(dcBackup()) << "Failed to start" << description << "command: influxd" << args.join(' ') << process.errorString();
        return false;
    }

    if (!process.waitForFinished(-1)) {
        qCWarning(dcBackup()) << description << "did not finish normally. Command: influxd" << args.join(' ') << process.errorString();
        return false;
    }

    const QByteArray standardOutput = process.readAllStandardOutput();
    const QByteArray standardError = process.readAllStandardError();
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        qCWarning(dcBackup()) << description << "failed with exit code" << process.exitCode() << "command: influxd" << args.join(' ');
        if (!standardOutput.isEmpty())
            qCWarning(dcBackup()) << description << "stdout:" << standardOutput;
        if (!standardError.isEmpty())
            qCWarning(dcBackup()) << description << "stderr:" << standardError;
        return false;
    }

    if (!standardOutput.isEmpty())
        qCDebug(dcBackup()) << description << "stdout:" << standardOutput;
    if (!standardError.isEmpty())
        qCDebug(dcBackup()) << description << "stderr:" << standardError;

    return true;
}

bool BackupManager::createInfluxBackup(const QString &destinationPath) const
{
    if (!QDir().mkpath(destinationPath)) {
        qCWarning(dcBackup()) << "Failed to create InfluxDB backup directory:" << destinationPath;
        return false;
    }

    QStringList args;
    args << "backup" << "-portable" << "-host" << influxBackupHostArgument()
         << "-db" << m_influxDatabaseName << destinationPath;

    qCInfo(dcBackup()) << "Creating InfluxDB backup for database" << m_influxDatabaseName << "in" << destinationPath;
    return runInfluxdCommand(args, "Creating InfluxDB backup");
}

bool BackupManager::dropInfluxDatabase() const
{
    return executeInfluxQuery(createInfluxQueryRequest(QString("DROP DATABASE %1").arg(quotedInfluxDatabaseName())), nullptr);
}

bool BackupManager::restoreInfluxBackup(const QString &backupPath) const
{
    qCInfo(dcBackup()) << "Restoring InfluxDB backup for database" << m_influxDatabaseName << "from" << backupPath;

    if (!dropInfluxDatabase())
        return false;
    if (!waitForInfluxDatabaseRemoved())
        return false;

    QStringList args;
    args << "restore" << "-portable" << "-host" << influxBackupHostArgument()
         << "-db" << m_influxDatabaseName
         << backupPath;

    return runInfluxdCommand(args, "Restoring InfluxDB backup");
}

BackupFiles BackupManager::backupFiles(const QString &destinationDir, const QString &archivePrefix) const
{
    BackupFiles backupFiles;

    const QDir dir(destinationDir);
    if (!dir.exists())
        return backupFiles;

    const QString pattern = QString("%1-*.tar.gz").arg(archivePrefix);
    const QFileInfoList fileInfos = dir.entryInfoList({pattern}, QDir::Files | QDir::NoSymLinks, QDir::Name);
    for (const QFileInfo &fileInfo : fileInfos) {
        BackupFile backupFile;
        if (parseBackupFileInfo(fileInfo, archivePrefix, &backupFile)) {
            backupFiles.append(backupFile);
        }
    }

    std::sort(backupFiles.begin(), backupFiles.end(), [](const BackupFile &left, const BackupFile &right) {
        if (left.timestamp() != right.timestamp())
            return left.timestamp() > right.timestamp();

        return left.fileName() > right.fileName();
    });

    return backupFiles;
}

bool BackupManager::createBackup(const QString &sourceDir, const QString &destinationDir, int maxBackups, const QString &archivePrefix, QString *archivePath)
{
    qCInfo(dcBackup()) << "Creating a backup from" << sourceDir;
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
    const QString uniqueSuffix = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
    const QString archiveBaseName = QString("%1-%2-%3-%4.tar.gz").arg(archivePrefix, QString::fromLatin1(NYMEA_VERSION_STRING), timestamp, uniqueSuffix);
    const QString createdArchivePath = QDir(destinationDir).filePath(archiveBaseName);

    QString archiveSourcePath = QDir(sourceDir).absolutePath();
    QScopedPointer<QTemporaryDir> stagedBackupDirectory;
    if (m_influxBackupEnabled) {
        stagedBackupDirectory.reset(new QTemporaryDir(QDir::tempPath() + QDir::separator() + "nymea-backup-XXXXXX"));
        if (!stagedBackupDirectory->isValid()) {
            qCWarning(dcBackup()) << "Failed to create temporary backup staging directory.";
            return false;
        }

        archiveSourcePath = stagedBackupDirectory->path();
        if (!copyDirectoryContents(sourceDir, archiveSourcePath))
            return false;

        const InfluxDatabaseState databaseState = checkInfluxDatabaseState();
        if (databaseState == InfluxDatabaseStateError) {
            qCWarning(dcBackup()) << "Failed to create complete backup because InfluxDB database state could not be checked.";
            return false;
        }

        if (databaseState == InfluxDatabaseStateMissing) {
            qCInfo(dcBackup()) << "Skipping InfluxDB backup because database does not exist:" << m_influxDatabaseName;
        } else if (!createInfluxBackup(QDir(archiveSourcePath).filePath(influxBackupDirectory))) {
            qCWarning(dcBackup()) << "Failed to create complete backup because InfluxDB backup failed.";
            return false;
        }
    }

    qCInfo(dcBackup()) << "Writing backup file" << createdArchivePath;
    QStringList args;
    args << "-czf" << createdArchivePath << "-C" << archiveSourcePath << ".";

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

    qCInfo(dcBackup()) << "Backup archive file written successfully" << createdArchivePath;

    if (archivePath)
        *archivePath = createdArchivePath;

    if (QDir(destinationDir).absolutePath() == QDir(m_destinationDirectory).absolutePath())
        updateBackupDestinationDirectoryWatcher();

    if (maxBackups > 0) {
        const QString pattern = QString("%1-*.tar.gz").arg(archivePrefix);
        QFileInfoList files = dst.entryInfoList({pattern}, QDir::Files | QDir::NoSymLinks,
                                                QDir::Time); // sorted by time (newest first)
        if (files.size() <= maxBackups) {
            emitBackupFilesChangedIfNeeded();
            return true;
        }

        const QString createdArchiveAbsolutePath = QFileInfo(createdArchivePath).absoluteFilePath();
        int remainingFiles = files.size();

        // Delete the oldest backups first but never the archive we just created.
        for (int i = files.size() - 1; i >= 0 && remainingFiles > maxBackups; --i) {
            const QString path = files.at(i).absoluteFilePath();
            if (path == createdArchiveAbsolutePath)
                continue;

            if (!QFile::remove(path)) {
                qCWarning(dcBackup()) << "Warning: failed to remove old backup: " << path;
            } else {
                qCDebug(dcBackup()) << "Removed old backup: " << path;
                --remainingFiles;
            }
        }
    }

    emitBackupFilesChangedIfNeeded();
    return true;
}

bool BackupManager::restoreBackup(const QString &fileName, const QString &destinationDir, bool safetyBackup)
{
    qCInfo(dcBackup()) << "Restoring backup from" << fileName;

    QFileInfo arcInfo(fileName);
    if (!arcInfo.exists() || !arcInfo.isFile()) {
        qCWarning(dcBackup()) << "Archive not found:" << fileName;
        return false;
    }

    const QString destinationPath = QDir(destinationDir).absolutePath();
    const QString archivePath = arcInfo.absoluteFilePath();
    const QString archiveParentPath = arcInfo.absoluteDir().absolutePath();
    if (archiveParentPath == destinationPath || archiveParentPath.startsWith(destinationPath + QDir::separator())) {
        qCWarning(dcBackup()) << "Refusing to restore from archive stored inside target directory:" << archivePath << destinationPath;
        return false;
    }

    if (!validateBackupArchive(archivePath)) {
        qCWarning(dcBackup()) << "Backup archive validation failed:" << archivePath;
        return false;
    }

    const QString restoreId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QString stagedDirectoryPath = destinationPath + ".restore-" + restoreId;
    if (!QDir().mkpath(stagedDirectoryPath)) {
        qCWarning(dcBackup()) << "Failed to create staging directory for restore:" << stagedDirectoryPath;
        return false;
    }

    const QString stagedTargetPath = QDir(stagedDirectoryPath).absolutePath();
    QStringList args;
    args << "-xzf" << archivePath << "-C" << stagedTargetPath;

    qCInfo(dcBackup()) << "Extracting backup" << fileName << "into" << stagedTargetPath;
    int exitCode = QProcess::execute("tar", args);
    if (exitCode != 0) {
        qCWarning(dcBackup()) << "tar failed with exit code" << exitCode << "during restore. Command: tar" << args.join(' ');
        QDir(stagedDirectoryPath).removeRecursively();
        return false;
    }

    const QString stagedInfluxBackupPath = QDir(stagedTargetPath).filePath(influxBackupDirectory);
    if (QFileInfo::exists(stagedInfluxBackupPath)) {
        if (!QFileInfo(stagedInfluxBackupPath).isDir()) {
            qCWarning(dcBackup()) << "Invalid InfluxDB backup path in archive:" << stagedInfluxBackupPath;
            QDir(stagedDirectoryPath).removeRecursively();
            return false;
        }

        readInfluxDatabaseConfiguration(stagedTargetPath);
        if (!restoreInfluxBackup(stagedInfluxBackupPath)) {
            qCWarning(dcBackup()) << "Failed to restore InfluxDB backup from archive:" << stagedInfluxBackupPath;
            qCWarning(dcBackup()) << "Continuing restore without InfluxDB history.";
            dropInfluxDatabase();
            waitForInfluxDatabaseRemoved();
        }

        const QString stagedMetadataPath = QDir(stagedTargetPath).filePath(influxBackupMetadataDirectory);
        if (!QDir(stagedMetadataPath).removeRecursively()) {
            qCWarning(dcBackup()) << "Failed to remove backup metadata from staged restore:" << stagedMetadataPath;
            QDir(stagedDirectoryPath).removeRecursively();
            return false;
        }
    }

    QFileInfo tgtInfo(destinationPath);
    QString backupDirectoryPath;
    if (tgtInfo.exists()) {
        if (!tgtInfo.isDir()) {
            qCWarning(dcBackup()) << "Target exists and is not a directory:" << destinationPath;
            QDir(stagedDirectoryPath).removeRecursively();
            return false;
        }

        backupDirectoryPath = destinationPath + ".bak-" + QDateTime::currentDateTimeUtc().toString("yyyyMMddHHmmss");
        if (!QDir().rename(destinationPath, backupDirectoryPath)) {
            qCWarning(dcBackup()) << "Failed to move existing target to backup:" << backupDirectoryPath;
            QDir(stagedDirectoryPath).removeRecursively();
            return false;
        }
    }

    if (!QDir().rename(stagedDirectoryPath, destinationPath)) {
        qCWarning(dcBackup()) << "Failed to move restored data into place:" << stagedDirectoryPath << destinationPath;
        if (!backupDirectoryPath.isEmpty() && !QDir().rename(backupDirectoryPath, destinationPath))
            qCWarning(dcBackup()) << "Failed to roll back previous settings directory:" << backupDirectoryPath << destinationPath;

        QDir(stagedDirectoryPath).removeRecursively();
        return false;
    }

    if (!backupDirectoryPath.isEmpty() && !safetyBackup && !QDir(backupDirectoryPath).removeRecursively())
        qCWarning(dcBackup()) << "Failed to remove previous settings backup after successful restore:" << backupDirectoryPath;

    qCInfo(dcBackup()) << "Restored backup" << fileName << "successfully into" << destinationPath;
    return true;
}

void BackupManager::reevaluateAutomaticBackup()
{
    m_automaticBackupTimer->stop();

    if (!m_automaticBackupEnabled || m_sourceDirectory.isEmpty() || m_destinationDirectory.isEmpty())
        return;

    const BackupFiles files = backupFiles(m_destinationDirectory);
    if (files.isEmpty()) {
        triggerAutomaticBackup();
        return;
    }

    const QDateTime nextBackup = files.first().timestamp().toUTC().addMSecs(automaticBackupIntervalMs());
    const qint64 delayMs = QDateTime::currentDateTimeUtc().msecsTo(nextBackup);
    if (delayMs <= 0) {
        triggerAutomaticBackup();
        return;
    }

    m_automaticBackupTimer->start(static_cast<int>(qMin(delayMs, static_cast<qint64>(std::numeric_limits<int>::max()))));
}

void BackupManager::triggerAutomaticBackup()
{
    if (!m_automaticBackupEnabled)
        return;

    QString archivePath;
    if (createBackup(m_sourceDirectory, m_destinationDirectory, m_maxBackups, "nymea-configuration", &archivePath)) {
        qCInfo(dcBackup()) << "Created automatic backup:" << archivePath;
        reevaluateAutomaticBackup();
        return;
    }

    qCWarning(dcBackup()) << "Failed to create automatic backup. Retrying later.";
    m_automaticBackupTimer->start(static_cast<int>(qMin(automaticBackupIntervalMs(), static_cast<qint64>(std::numeric_limits<int>::max()))));
}

qint64 BackupManager::automaticBackupIntervalMs() const
{
    return static_cast<qint64>(m_automaticBackupInterval) * 60 * 60 * 1000;
}

void BackupManager::updateBackupDestinationDirectoryWatcher()
{
    const QString watchedDirectory = m_destinationDirectory.isEmpty() ? QString() : QDir(m_destinationDirectory).absolutePath();

    const QStringList watchedDirectories = m_backupDestinationDirectoryWatcher->directories();
    foreach (const QString &directory, watchedDirectories) {
        if (directory != watchedDirectory) {
            m_backupDestinationDirectoryWatcher->removePath(directory);
        }
    }

    if (watchedDirectory.isEmpty())
        return;

    if (!QDir(watchedDirectory).exists())
        return;

    if (!m_backupDestinationDirectoryWatcher->directories().contains(watchedDirectory)) {
        m_backupDestinationDirectoryWatcher->addPath(watchedDirectory);
    }
}

void BackupManager::emitBackupFilesChangedIfNeeded()
{
    const BackupFiles currentBackupFiles = backupFiles(m_destinationDirectory);
    if (currentBackupFiles == m_backupFiles)
        return;

    qCDebug(dcBackup()) << "Backup files changed";
    m_backupFiles = currentBackupFiles;
    emit backupFilesChanged();
}

void BackupManager::onBackupDestinationDirectoryChanged(const QString &path)
{
    qCDebug(dcBackup()) << "Backup destination directory changed:" << path;
    updateBackupDestinationDirectoryWatcher();
    emitBackupFilesChangedIfNeeded();
}
