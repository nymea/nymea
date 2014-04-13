#ifndef BOBCLIENT_H
#define BOBCLIENT_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QColor>
#include <QTime>

class BobClient : public QObject
{
    Q_OBJECT
public:
    explicit BobClient(QObject *parent = 0);
    bool connect(const QString &hostname, int port);
    bool connected() const;

    int lightsCount();
    QColor currentColor(int channel);

    void setPriority(int priority);
    
signals:
    
public slots:
    void setColor(int channel, QColor color);
    void sync();

private:
    QMap<int, QColor> m_colors; //channel, color
    void *m_boblight;
    bool m_connected;
    QString m_hostname;
    int m_port;

    QTime m_lastSyncTime;
    QTimer m_resyncTimer;
};

#endif // BOBCLIENT_H
