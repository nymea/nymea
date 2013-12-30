#include "logwriter.h"
#include <QDebug>
#include <QDateTime>

LogWriter::LogWriter(QObject *parent) :
    QObject(parent)
{
}

void LogWriter::write(LogWriter::MessageType messageType, QString logMessage)
{
    QString timeStamp = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm");;
    qDebug() << timeStamp;

}
