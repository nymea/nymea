#include "jsonhandler.h"

#include <QJsonDocument>
#include <QVariantMap>
#include <QDebug>

JsonHandler::JsonHandler(QObject *parent) :
    QObject(parent)
{

}

QByteArray JsonHandler::process(const QByteArray &data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    if(error.error != QJsonParseError::NoError) {
        qDebug() << "failed to parse data" << data << ":" << error.errorString();
    }




}
