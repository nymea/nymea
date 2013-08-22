#ifndef HIVECLIENTCORE_H
#define HIVECLIENTCORE_H

#include <QObject>
#include "qtquick2applicationviewer.h"
#include "client.h"

class HiveClientCore : public QObject
{
    Q_OBJECT
public:
    explicit HiveClientCore(QObject *parent = 0);
    
private:
    Client *m_client;


    QtQuick2ApplicationViewer *m_viewer;

signals:
    
public slots:
    
};

#endif // HIVECLIENTCORE_H
