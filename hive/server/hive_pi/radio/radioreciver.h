#ifndef RADIORECIVER_H
#define RADIORECIVER_H

#include <QObject>

class RadioReciver : public QObject
{
    Q_OBJECT
public:
    explicit RadioReciver(QObject *parent = 0);
    
private:
    static void handleInterrupt();

signals:
    
public slots:
    
};

#endif // RADIORECIVER_H
