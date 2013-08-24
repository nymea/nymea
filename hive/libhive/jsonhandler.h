#ifndef JSONHANDLER_H
#define JSONHANDLER_H

#include <QObject>


class JsonHandler : public QObject
{
    Q_OBJECT
public:
    explicit JsonHandler(QObject *parent = 0);
    
private:


signals:
    void notifyAll(const QByteArray &data);

    
public slots:
    QByteArray process(const QByteArray &data);

    
};

#endif // JSONHANDLER_H
