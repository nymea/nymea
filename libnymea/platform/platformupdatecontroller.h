#ifndef PLATFORMUPDATECONTROLLER_H
#define PLATFORMUPDATECONTROLLER_H

#include <QObject>

class PlatformUpdateController : public QObject
{
    Q_OBJECT
public:
    explicit PlatformUpdateController(QObject *parent = nullptr);
    virtual ~PlatformUpdateController() = default;

    virtual bool updateManagementAvailable();

    virtual QString currentVersion() const;
    virtual QString candidateVersion() const;

//    virtual QMap<QString, QString> changelog() const = 0;

    virtual void checkForUpdates();
    virtual bool updateAvailable() const;
    virtual bool startUpdate();

    virtual bool rollbackAvailable() const;
    virtual bool startRollback();

    virtual bool updateInProgress() const;

    virtual QStringList availableChannels() const;
    virtual QString currentChannel() const;
    virtual bool selectChannel(const QString &channel);

signals:
    void updateStatusChanged();
};

#endif // PLATFORMUPDATECONTROLLER_H
