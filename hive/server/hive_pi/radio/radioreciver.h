#ifndef RADIORECIVER_H
#define RADIORECIVER_H

#define RC_MAX_CHANGES 50

#include <QObject>

class RadioReciver : public QObject
{
    Q_OBJECT
public:
    explicit RadioReciver(QObject *parent = 0);
    
    enum Frequency{
        RF433MHz = 0x0,
        RF868MHz = 0x1
    };

private:
    void handleInterrupt();
    static void handleRC433Interrupt();
    static void handleRC868Interrupt();

    static void detectProtocol(int signalCount);
    static float parseTemperature(QByteArray codeBin);

    int m_pin433;
    int m_pin868;

signals:
    
public slots:
    void enableReceiver();
    void disableReceiver();
    void setFrequency(Frequency frequency);

    
};

#endif // RADIORECIVER_H
