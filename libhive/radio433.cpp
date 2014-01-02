#include "radio433.h"
#include <QDebug>
#include <sys/time.h>


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

    connect(m_receiver,SIGNAL(pinInterrupt()),this,SLOT(handleInterrupt()));

    m_receiver->start();
}

Radio433::~Radio433()
{
    m_receiver->quit();
    m_receiver->wait();
}

void Radio433::sendData(QList<int> rawData)
{

    qDebug() << "send 433";
    //first we have to disable our receiver, to prevent reading this signal
    //m_receiver->stop();

    m_transmitter->setValue(LOW);
    delayMicroseconds(500);

    int flag=1;
    foreach (int delay, rawData) {
        // 1 = High, 0 = Low
        m_transmitter->setValue(flag++ %2);
        delayMicroseconds(delay);
    }

    // re-enable it
    //m_receiver->start();

}

int Radio433::micros()
{
    struct timeval tv ;
    int now ;

    gettimeofday (&tv, NULL) ;
    now  = (int)tv.tv_sec * (int)1000000 + (int)tv.tv_usec ;

    return (int)(now - m_epochMicro) ;
}

void Radio433::delayMicroseconds(int pulseLength)
{
    struct timespec sleeper ;

    if(pulseLength <= 0){
        return;
    }else {
        if(pulseLength < 100){
            struct timeval tNow, tLong, tEnd ;

            gettimeofday (&tNow, NULL) ;
            tLong.tv_sec  = pulseLength / 1000000 ;
            tLong.tv_usec = pulseLength % 1000000 ;
            timeradd (&tNow, &tLong, &tEnd) ;

            while (timercmp (&tNow, &tEnd, <)){
                gettimeofday (&tNow, NULL) ;
            }
        }
        sleeper.tv_sec  = 0 ;
        sleeper.tv_nsec = (long)(pulseLength * 1000) ;
        nanosleep (&sleeper, NULL) ;
    }
}


void Radio433::handleInterrupt()
{
    long currentTime = micros();
    m_duration = currentTime - m_lastTime;

    // filter nois
    if (m_duration > 5000 && m_duration > m_timings[0] - 200 && m_duration < m_timings[0] + 200) {

        m_repeatCount++;
        m_changeCount--;

        if(m_repeatCount == 2) {

            // if we have a regular signal (1 bit sync + 48 bit data)
            if(m_changeCount == RC_MAX_CHANGES){
                // write rawdata to a List and reset values to 0
                QList<int> rawData;
                for(int i = 0; i < RC_MAX_CHANGES; i++ ){
                    rawData.append(m_timings[i]);
                    m_timings[i] = 0;
                }
//                qDebug() << "-----------------------------------------------------------";
//                qDebug() << "|                    GENERIC signal                       |";
//                qDebug() << "-----------------------------------------------------------";
//                qDebug() << "delay      :" << rawData.first() /31;
//                qDebug() << rawData;

                emit dataReceived(rawData);
            }
            m_repeatCount = 0;
        }
        m_changeCount = 0;

    }else if(m_duration > 5000){
        m_changeCount = 0;
    }
    if (m_changeCount > RC_MAX_CHANGES) {
        m_changeCount = 0;
        m_repeatCount = 0;
    }
    m_timings[m_changeCount++] = m_duration;
    m_lastTime = currentTime;
}
