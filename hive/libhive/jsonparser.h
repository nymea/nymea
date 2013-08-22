#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <QObject>
//#include <qjson/serializer.h>

class JsonParser : public QObject
{
    Q_OBJECT
public:
    explicit JsonParser(QObject *parent = 0);
    
signals:
    
public slots:
    
};

#endif // JSONPARSER_H
