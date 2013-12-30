#include "radioswitch.h"
#include <QDebug>
#include <QStringList>


RadioSwitch::RadioSwitch(QObject *parent) :
    RadioPlugin(parent)
{
    m_delay = 0;
    m_binCode = 0;
}

QByteArray RadioSwitch::getBinCode()
{
    if(m_binCode.isEmpty()){
        return NULL;
    }else{
        return m_binCode;
    }
}

bool RadioSwitch::isValid(QList<int> rawData)
{
    m_delay = rawData.first()/31;
    QByteArray binCode;

    // new Remote -> average 314
    if(m_delay > 300 && m_delay < 400){
        // go trough all 48 timings (without sync signal)
        for(int i = 1; i <= 48; i+=2 ){
            int div;
            int divNext;

            // if short
            if(rawData.at(i) < 700){
                div = 1;
            }else{
                div = 3;
            }
            // if long
            if(rawData.at(i+1) < 700){
                divNext = 1;
            }else{
                divNext = 3;
            }

            //              _
            // if we have  | |___ = 0 -> in 4 delays => 1000
            //                 _
            // if we have  ___| | = 1 -> in 4 delays => 0001

            if(div == 1 && divNext == 3){
                binCode.append('0');
            }else if(div == 3 && divNext == 1){
                binCode.append('1');
            }else{
                return false;
            }
        }
    }


    m_binCode = binCode;

    // get the channel of the remote signal (5 channels, 1=on, 0 = off)
    QByteArray channelSettings;
    for(int i = 1; i < 10; i+=2){
        if(m_binCode.at(i-1) == '0' && m_binCode.at(i) == '1'){
            channelSettings.append("0");
        }else{
            channelSettings.append("1");
        }
    }
    // get the button letter
    char button;
    if(m_binCode.at(10) == '0' && m_binCode.at(11) == '0'){
        button = 'A';
    }
    if(m_binCode.at(12) == '0' && m_binCode.at(13) == '0'){
        button = 'B';
    }
    if(m_binCode.at(14) == '0' && m_binCode.at(15) == '0'){
        button = 'C';
    }
    if(m_binCode.at(16) == '0' && m_binCode.at(17) == '0'){
        button = 'D';
    }
    if(m_binCode.at(18) == '0' && m_binCode.at(19) == '0'){
        button = 'E';
    }

    QStringList byteList;
    for(int i = 4; i <= 24; i+=4){
        byteList.append(binCode.left(4));
        binCode = binCode.right(binCode.length() -4);
    }

    bool buttonStatus;
    if(byteList.last().toInt(0,2) == 1){
        buttonStatus = true;
    }else
        if(byteList.last().toInt(0,2) == 4){
            buttonStatus = false;
        }else{
            return false;
        }

    qDebug() << "-----------------------------------------------------------";
    qDebug() << "|                     REMOTE signal                       |";
    qDebug() << "-----------------------------------------------------------";
    qDebug() << "delay      :" << m_delay;
    qDebug() << "bin CODE   :" << m_binCode;
    qDebug() << byteList;
    qDebug() << "Channels:" << channelSettings << "Button:" << button << "=" << buttonStatus;
    emit switchSignalReceived(channelSettings,button,buttonStatus);
    return true;
}

QByteArray RadioSwitch::calcBinCode(const QByteArray &channel, const RadioSwitch::RadioRemoteButton &button, const bool &buttonStatus)
{
    QByteArray binCode;

    // channels
    for(int i = 0; i < channel.length(); i++){
        if(channel.at(i) == '0'){
            binCode.append("01");
        }else{
            binCode.append("00");
        }
    }

    // Buttons
    switch (button) {
    case RadioSwitch::A :
        binCode.append("0001010101");
        break;
    case RadioSwitch::B:
        binCode.append("0100010101");
        break;
    case RadioSwitch::C:
        binCode.append("0101000101");
        break;
    case RadioSwitch::D:
        binCode.append("0101010001");
        break;
    case RadioSwitch::E:
        binCode.append("0101010100");
        break;
    default:
        break;
    }

    // ON/OFF
    if(buttonStatus){
        binCode.append("0001");
    }else{
        binCode.append("0100");
    }

    return binCode;
}
