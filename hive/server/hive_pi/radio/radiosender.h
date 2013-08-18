#ifndef RADIOSENDER_H
#define RADIOSENDER_H

#include <QObject>

class RadioSender : public QObject
{
    Q_OBJECT
public:
    explicit RadioSender(QObject *parent = 0);
    
private:
    // [us]
    int m_pulseLength;

    void sendSync();
    void transmit(int high, int low);

signals:
    
public slots:
    void sendBin(QString codeBin);
    void setPulseLength(int pulseLength);
    
};

#endif // RADIOSENDER_H
