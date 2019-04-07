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

    virtual QString currentVersion() const = 0;
    virtual QString candidateVersion() const = 0;

//    virtual QMap<QString, QString> changelog() const = 0;

    virtual void checkForUpdates() = 0;
    virtual bool updateAvailable() const = 0;
    virtual bool startUpdate() = 0;

    virtual bool rollbackAvailable() const = 0;
    virtual bool startRollback() = 0;

    virtual bool updateInProgress() const = 0;

    virtual QStringList channels() const = 0;
    virtual QString currentChannel() const = 0;
    virtual bool selectChannel(const QString &channel) = 0;

signals:
    void updateStatusChanged();
};

#endif // PLATFORMUPDATECONTROLLER_H
