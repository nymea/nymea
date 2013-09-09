#ifndef RADIOSENDER_H
#define RADIOSENDER_H

#include <QObject>

class RadioSender : public QObject
{
    Q_OBJECT
public:
    explicit RadioSender(QObject *parent = 0);
    
    enum Frequency{
        RF433MHz = 0x0,
        RF868MHz = 0x1
    };

    enum LineCode{
        UNIPOLAR = 0x2,
        MANCHESTER = 0x3,
        DMANCHESTER = 0x4,
        SWITCH = 0x5,
        THERMOMETER = 0x6,
        WEATHERSTATION = 0x7
    };


private:
    // [us = micro seconds]
    int m_pulseLength;

    Frequency m_frequenze;
    LineCode m_lineCode;
    int m_pin;

    void sendSync();
    void send0();
    void send1();

signals:
    
public slots:
    void sendBin(QByteArray codeBin);
    void setFrequency(Frequency frequency);
    void setLineCode(LineCode lineCode);
    void setPulseLength(int pulseLength);
};

#endif // RADIOSENDER_H
