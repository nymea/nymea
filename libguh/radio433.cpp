/*!
  \class Radio433
  \brief The Radio433 class helps to interact with the 433 MHz Receiver and Transmitter.

  \l{http://tech.jolowe.se/home-automation-rf-protocols/}

  \inmodule libguh

*/

#include "radio433.h"
#include <QDebug>
#include <sys/time.h>


/*! Constructs a \l{Radio433} object with the given \a parent and initializes the \l{Gpio} pins
 * for the transmitter and the receiver.
 */
Radio433::Radio433(QObject *parent) :
    QObject(parent)
{
    // Set up receiver
    m_receiver = new Gpio(this,27);
    m_receiver->setDirection(INPUT);
    m_receiver->setEdgeInterrupt(EDGE_BOTH);

    // Set up transmitter
    m_transmitter = new Gpio(this,22);
    m_transmitter->setDirection(OUTPUT);
    m_transmitter->setValue(LOW);

    connect(m_receiver, &Gpio::pinInterrupt, this, &Radio433::handleInterrupt);

    m_receiver->start();
}

/*! Destroyes the \l{Radio433} object and stops the running threads. */
Radio433::~Radio433()
{
    m_receiver->quit();
    m_receiver->wait();
}

/*! Sends the given \a rawData over the transmitter. The sync signal has to be already in the
 * \a rawData.
 */
void Radio433::sendData(QList<int> rawData)
{

    //first we have to disable our receiver, to prevent reading this signal
    m_receiver->stop();

    m_transmitter->setValue(LOW);
    int flag=1;

    for(int i = 0; i <= 8; i++){
        foreach (int delay, rawData) {
            // 1 = High, 0 = Low
            m_transmitter->setValue(flag %2);
            flag++;
            delayMicros(delay);
        }
    }
    m_transmitter->setValue(LOW);
    // re-enable it
    m_transmitter->setValue(LOW);
    m_receiver->start();

}

/*! Returns the current system time in microseconds. */
int Radio433::micros()
{
    struct timeval tv ;
    int now ;

    gettimeofday (&tv, NULL) ;
    now  = (int)tv.tv_sec * (int)1000000 + (int)tv.tv_usec ;

    return (int)(now - m_epochMicro) ;
}

/*! Creates a delay of a certain time (\a microSeconds). */
void Radio433::delayMicros(int microSeconds)
{
    struct timespec sleeper;

    sleeper.tv_sec  = 0;
    sleeper.tv_nsec = (long)(microSeconds * 1000);

    nanosleep (&sleeper, NULL) ;
}

/*! This method handels an interrupt on the receiver pin and recognizes, if a valid message of
 *  48 bit or 64 bit was received.
 */
void Radio433::handleInterrupt()
{
    long currentTime = micros();
    m_duration = currentTime - m_lastTime;

    // filter nois
    if (m_duration > 5000 && m_duration > m_timings[0] - 200 && m_duration < m_timings[0] + 200) {

        m_repeatCount++;
        m_changeCount--;

        if(m_repeatCount == 2) {
            // if we have a regular signal (1 bit sync + 48 bit data or 1bit sync + 64 bit data)
            if(m_changeCount == 49 || m_changeCount == 65){
                QList<int> rawData;

                switch (m_changeCount) {
                case 49:
                    // write rawdata to a List and reset values to 0
                    for(int i = 0; i < 49; i++ ){
                        rawData.append(m_timings[i]);
                        m_timings[i] = 0;
                    }
//                    qDebug() << "-----------------------------------------------------------";
//                    qDebug() << "|                    GENERIC signal                       |";
//                    qDebug() << "-----------------------------------------------------------";
//                    qDebug() << "signal length  :" << 49;
//                    qDebug() << "delay      :" << rawData.first() /31;
//                    qDebug() << rawData;

                    emit dataReceived(rawData);
                    break;

                case 65:
                    // write rawdata to a List and reset values to 0
                    for(int i = 0; i < 65; i++ ){
                        rawData.append(m_timings[i]);
                        m_timings[i] = 0;
                    }
                    qDebug() << "-----------------------------------------------------------";
                    qDebug() << "|                    GENERIC signal                       |";
                    qDebug() << "-----------------------------------------------------------";
                    qDebug() << "signal length  :" << 65;
                    qDebug() << "delay          :" << rawData.first() /10;
                    qDebug() << rawData;

                    emit dataReceived(rawData);
                default:
                    break;
                }
            }
            m_repeatCount = 0;
        }
        m_changeCount = 0;

    }else if(m_duration > 5000){
        m_changeCount = 0;
    }
    if (m_changeCount >= RC_MAX_CHANGES) {
        m_changeCount = 0;
        m_repeatCount = 0;
    }
    m_timings[m_changeCount++] = m_duration;
    m_lastTime = currentTime;
}
