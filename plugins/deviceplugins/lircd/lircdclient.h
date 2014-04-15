#ifndef LIRCCLIENT_H
#define LIRCCLIENT_H

#include <QObject>
#include <QLocalSocket>
#include <QStringList>

class LircClient : public QObject
{
    Q_OBJECT
public:
    explicit LircClient(QObject *parent = 0);

    bool connect();

signals:
    void buttonPressed(const QString &remoteName, const QString &buttonName, int repeat);

private slots:
    void readyRead();

private:
    void readRemotes();

private:
    QLocalSocket *m_socket;

    QStringList m_remotes;
};

#endif // LIRCCLIENT_H
