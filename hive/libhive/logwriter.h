#ifndef LOGWRITER_H
#define LOGWRITER_H

#include <QObject>
#include <QFile>

class LogWriter : public QObject
{
    Q_OBJECT
public:
    explicit LogWriter(QObject *parent = 0);
    enum MessageType{
        INFO = 0x1,
        ERROR = 0x2,
        TEMP = 0X4
    };



signals:
    
public slots:
    void write(MessageType messageType, QString logMessage);
    
};

#endif // LOGWRITER_H
