/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                   *
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

#include "debugreportgenerator.h"
#include "loggingcategories.h"
#include "nymeasettings.h"
#include "nymeacore.h"

#include <QDir>
#include <QFile>
#include <QTimer>
#include <QDateTime>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QProcessEnvironment>

namespace nymeaserver {

DebugReportGenerator::DebugReportGenerator(QObject *parent) : QObject(parent)
{

}

DebugReportGenerator::~DebugReportGenerator()
{
    cleanupReport();
}

QByteArray DebugReportGenerator::reportFileData() const
{
    return m_reportFileData;
}

QString DebugReportGenerator::reportFileName()
{
    return m_reportFileName;
}

QString DebugReportGenerator::md5Sum() const
{
    return m_md5Sum;
}

void DebugReportGenerator::generateReport()
{
    qCDebug(dcDebugServer()) << "Start generating debug report";
    m_reportFileName = QDateTime::currentDateTime().toString("yyyyMMddhhmm") + "-nymea-debug-report";

    m_reportDirectory.setPath(QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::TempLocation)).arg(m_reportFileName));
    //m_reportDirectory.setPath(QString("/tmp/%1").arg(m_reportFileName));
    if (!m_reportDirectory.exists()) {
        qCDebug(dcDebugServer()) << "Create temporary folder to collect the data" << m_reportDirectory.path();
        if (!m_reportDirectory.mkpath(m_reportDirectory.path())) {
            qCWarning(dcDebugServer()) << "Could not create output directory for debug report";
            emit finished(false);
            return;
        }
        m_reportDirectory.mkpath(m_reportDirectory.path() + "/config");
        m_reportDirectory.mkpath(m_reportDirectory.path() + "/network");
        m_reportDirectory.mkpath(m_reportDirectory.path() + "/logs");
    }

    m_reportFileName += ".tag.gz";

    saveConfigs();
    saveLogFiles();
    saveEnv();

    QProcess *pingProcess = new QProcess(this);
    pingProcess->setProcessChannelMode(QProcess::MergedChannels);
    connect(pingProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onPingProcessFinished(int,QProcess::ExitStatus)));

    QProcess *digProcess = new QProcess(this);
    digProcess->setProcessChannelMode(QProcess::MergedChannels);
    connect(digProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onDigProcessFinished(int,QProcess::ExitStatus)));

    QProcess *tracePathProcess = new QProcess(this);
    tracePathProcess->setProcessChannelMode(QProcess::MergedChannels);
    connect(tracePathProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onTracePathProcessFinished(int,QProcess::ExitStatus)));

    m_runningProcesses.append(pingProcess);
    m_runningProcesses.append(digProcess);
    m_runningProcesses.append(tracePathProcess);

    pingProcess->start("ping", { "-c", "4", "nymea.io" } );
    digProcess->start("dig", { "nymea.io" } );
    tracePathProcess->start("tracepath", { "nymea.io" } );
}

void DebugReportGenerator::copyFileToReportDirectory(const QString &fileName, const QString &subDirectory)
{
    QFileInfo fileInfo(fileName);
    if (fileInfo.exists()) {
        QString destination = m_reportDirectory.path() + "/" + subDirectory;
        if (!QFile::copy(fileName, destination + "/" + fileInfo.fileName())) {
            qCWarning(dcDebugServer()) << "Could not copy file" << fileName << "to" << destination;
        } else {
            qCDebug(dcDebugServer()) << "Copy file" << fileName << "-->" << destination;
        }
    }
}

void DebugReportGenerator::verifyRunningProcessesFinished()
{
    if (m_runningProcesses.isEmpty()) {
        qCDebug(dcDebugServer()) << "All async processes are finished. Start compressing the file.";
        m_compressProcess = new QProcess(this);
        m_compressProcess->setProcessChannelMode(QProcess::MergedChannels);
        m_compressProcess->setWorkingDirectory(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
        connect(m_compressProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onCompressProcessFinished(int, QProcess::ExitStatus)));
        m_compressProcess->start("tar", { "-zcf", m_reportFileName, "-C", QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/", m_reportDirectory.dirName() } );
        qCDebug(dcDebugServer()) << "Execut command" << m_compressProcess->program() << m_compressProcess->arguments();
    }
}

void DebugReportGenerator::saveLogFiles()
{
    QDir logDir("/var/log/");
    QStringList syslogFiles = logDir.entryList(QStringList() << "syslog*" << "nymea.*", QDir::Files);
    foreach (const QString &logFile, syslogFiles) {
        copyFileToReportDirectory(logDir.path() + "/" + logFile, "logs");
    }

}

void DebugReportGenerator::saveConfigs()
{
    // Start copy files setting files
    copyFileToReportDirectory(NymeaSettings(NymeaSettings::SettingsRoleGlobal).fileName(), "config");
    copyFileToReportDirectory(NymeaSettings(NymeaSettings::SettingsRoleDevices).fileName(), "config");
    copyFileToReportDirectory(NymeaSettings(NymeaSettings::SettingsRoleDeviceStates).fileName(), "config");
    copyFileToReportDirectory(NymeaSettings(NymeaSettings::SettingsRoleRules).fileName(), "config");
    copyFileToReportDirectory(NymeaSettings(NymeaSettings::SettingsRolePlugins).fileName(), "config");
    copyFileToReportDirectory(NymeaSettings(NymeaSettings::SettingsRoleTags).fileName(), "config");
    copyFileToReportDirectory(NymeaCore::instance()->configuration()->logDBName(), "config");
}

void DebugReportGenerator::saveEnv()
{
    QFile outputFile(m_reportDirectory.path() + "/env.txt");
    if (!outputFile.open(QIODevice::ReadWrite)) {
        qCWarning(dcDebugServer()) << "Could not open env file" << outputFile.fileName();
        return;
    }

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QTextStream stream(&outputFile);
    foreach(const QString &key, env.keys()) {
        qCDebug(dcDebugServer()) << "Process environment:" << key << "-->" << env.value(key);
        stream << key << "=" << env.value(key) << "\n";
    }
    outputFile.close();
}

void DebugReportGenerator::cleanupReport()
{
    QFile reportFile(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" + m_reportFileName);
    if (reportFile.exists()) {
        qCDebug(dcDebugServer()) << "Delete report file" << reportFile.fileName();
        if (!reportFile.remove()) {
            qCWarning(dcDebugServer()) << "Could not delete report file" << reportFile.fileName();
        }
    }

    if (m_reportDirectory.exists()) {
        qCDebug(dcDebugServer()) << "Clean up report directory" << m_reportDirectory.path();
        if (!m_reportDirectory.removeRecursively()) {
            qCWarning(dcDebugServer()) << "Could not delete report directory" << m_reportDirectory.path();
        }
    }
}

void DebugReportGenerator::onPingProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *process = static_cast<QProcess *>(sender());
    qCDebug(dcDebugServer()) << "Ping process finished" << exitCode << exitStatus;
    QByteArray processOutput = process->readAll();

    QFile outputFile(m_reportDirectory.path() + "/network/ping.txt");
    if (!outputFile.open(QIODevice::ReadWrite)) {
        qCWarning(dcDebugServer()) << "Could not open ping file" << outputFile.fileName();
    } else {
        qCDebug(dcDebugServer()) << "Write result into file" << outputFile.fileName();
        outputFile.write(processOutput);
        outputFile.close();
    }

    m_runningProcesses.removeAll(process);
    process->deleteLater();
    process = nullptr;

    verifyRunningProcessesFinished();
}

void DebugReportGenerator::onDigProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *process = static_cast<QProcess *>(sender());
    qCDebug(dcDebugServer()) << "Dig process finished" << exitCode << exitStatus;
    QByteArray processOutput = process->readAll();

    QFile outputFile(m_reportDirectory.path() + "/network/dns-lookup.txt");
    if (!outputFile.open(QIODevice::ReadWrite)) {
        qCWarning(dcDebugServer()) << "Could not open dig file" << outputFile.fileName();
    } else {
        qCDebug(dcDebugServer()) << "Write result into file" << outputFile.fileName();
        outputFile.write(processOutput);
        outputFile.close();
    }

    m_runningProcesses.removeAll(process);
    process->deleteLater();
    process = nullptr;

    verifyRunningProcessesFinished();
}

void DebugReportGenerator::onTracePathProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *process = static_cast<QProcess *>(sender());
    qCDebug(dcDebugServer()) << "Tracepath process finished" << exitCode << exitStatus;
    QByteArray processOutput = process->readAll();


    QFile outputFile(m_reportDirectory.path() + "/network/tracepath.txt");
    if (!outputFile.open(QIODevice::ReadWrite)) {
        qCWarning(dcDebugServer()) << "Could not open dig file" << outputFile.fileName();
    } else {
        qCDebug(dcDebugServer()) << "Write result into file" << outputFile.fileName();
        outputFile.write(processOutput);
        outputFile.close();
    }

    m_runningProcesses.removeAll(process);
    process->deleteLater();
    process = nullptr;

    verifyRunningProcessesFinished();
}

void DebugReportGenerator::onCompressProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *process = static_cast<QProcess *>(sender());
    qCDebug(dcDebugServer()) << "Compress process finished" << exitCode << exitStatus;

    qCDebug(dcDebugServer()) << "Clean up report directory" << m_reportDirectory.path();
    if (!m_reportDirectory.removeRecursively()) {
        qCWarning(dcDebugServer()) << "Could not delete report directory" << m_reportDirectory.path();
    }

    // Read the file
    QFile reportFile(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" + m_reportFileName);
    if (!reportFile.open(QIODevice::ReadOnly)) {
        qCWarning(dcDebugServer()) << "Could not open report file name for reading" << reportFile.fileName();
        emit finished(false);
    } else {
        m_reportFileData = reportFile.readAll();
        m_md5Sum =  QString::fromUtf8(QCryptographicHash::hash(m_reportFileData, QCryptographicHash::Md5).toHex());
        qCDebug(dcDebugServer()) << "File generated successfully" << reportFile.fileName() << m_reportFileData.size() << "B" << m_md5Sum;
        emit finished(true);
    }

    reportFile.close();

    // Todo: start expire timer
    QTimer::singleShot(30000, this, &DebugReportGenerator::timeout);

    process->deleteLater();
    process = nullptr;
}

}
