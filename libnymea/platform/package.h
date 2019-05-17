#ifndef PACKAGE_H
#define PACKAGE_H

#include <QString>

class Package
{
public:
    explicit Package(const QString &packageId = QString(), const QString &displayName = QString(), const QString &installedVersion = QString(), const QString &candidateVersion = QString(), const QString &changelog = QString());

    QString packageId() const;
    QString displayName() const;
    QString installedVersion() const;
    void setInstalledVersion(const QString &installedVersion);

    QString candidateVersion() const;
    void setCandidateVersion(const QString &candidateVersion);

    QString changelog() const;
    void setChangelog(const QString &changelog);

    bool updateAvailable() const;
    void setUpdateAvailable(bool updateAvailable);

    bool rollbackAvailable() const;
    void setRollbackAvailable(bool rollbackAvailable);

    bool canRemove() const;
    void setCanRemove(bool canRemove);

    bool operator==(const Package &other) const;
    bool operator!=(const Package &other) const;

private:
    QString m_packageId;
    QString m_displayName;
    QString m_installedVersion;
    QString m_candidateVersion;
    QString m_changeLog;

    bool m_updateAvailable = false;
    bool m_rollbackAvailable = false;
    bool m_canRemove = false;
};

#endif // PACKAGE_H
