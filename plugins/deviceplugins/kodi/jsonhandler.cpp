#include "jsonhandler.h"
#include "loggingcategories.h"

#include <QPixmap>
#include <QBuffer>

JsonHandler::JsonHandler(QObject *parent) :
    QObject(parent)
{
}

//QByteArray JsonHandler::createHelloMessage(QString title, QString message)
//{
//    QByteArray payload;

//    QByteArray iconData;
//    QBuffer buffer(&iconData);
//    buffer.open(QIODevice::WriteOnly);
//    icon.save(&buffer, "PNG");
//    //    payload.clear();

//    //    // titel
//    //    payload.push_back(QByteArray::fromStdString(title.toStdString()));
//    //    payload.push_back('\0');

//    //    // message
//    //    payload.push_back(QByteArray::fromStdString(message.toStdString()));
//    //    payload.push_back('\0');

//    //    // icontype ( 0=>NOICON, 1=>JPEG , 2=>PNG , 3=>GIF )
//    //    payload.push_back(2);

//    //    payload.push_back("0000");

//    //    // image data
//    //    payload.push_back(iconData);
//    //    payload.push_back('\0');

//    //    qCDebug(dcKodi) << payload;

//    return payload;
//}
