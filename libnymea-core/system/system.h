#ifndef SYSTEM_H
#define SYSTEM_H

#include <QObject>


namespace nymeaserver {

class Platform;

class System : public QObject
{
    Q_OBJECT

public:
    explicit System(Platform *platform, QObject *parent = nullptr);

    bool powerManagementAvailable() const;
    bool reboot();
    bool shutdown();

    bool updateManagementAvailable() const;
    bool updateAvailable() const;
    QString currentVersion() const;
    QString candidateVersion() const;
    QStringList availableChannels() const;
    QString currentChannel() const;
    bool selectChannel(const QString &channel) const;
    bool canUpdate() const;
    bool startUpdate();
    bool updateInProgress() const;

    bool rollbackAvailable() const;
    bool startRollback();

signals:
    void updateStatusChanged();

private:
    Platform *m_platform = nullptr;
};

}

#endif // SYSTEM_H
