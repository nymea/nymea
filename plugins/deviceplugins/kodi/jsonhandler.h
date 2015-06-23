#ifndef JSONHANDLER_H
#define JSONHANDLER_H

#include <QObject>

class JsonHandler : public QObject
{
    Q_OBJECT
public:
    explicit JsonHandler(QObject *parent = 0);

    //static QByteArray createHelloMessage(QString title, QString message);

signals:

public slots:

};

#endif // JSONHANDLER_H
