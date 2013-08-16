#include "radioreciver.h"
#include "wiringPi.h"
#include <QDebug>

RadioReciver::RadioReciver(QObject *parent) :
    QObject(parent)
{
    pinMode(2,INPUT);
    wiringPiISR(2, INT_EDGE_BOTH, &handleInterrupt);
}

void RadioReciver::handleInterrupt()
{
    qDebug() << "interrupt";
}
