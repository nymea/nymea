#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <QString>

class Repository
{
public:
    Repository();
    Repository(const QString &id, const QString &displayName, bool enabled);

    QString id() const;
    QString displayName() const;

    bool enabled() const;
    void setEnabled(bool enabled);

private:
    QString m_id;
    QString m_displayName;
    bool m_enabled = false;
};

#endif // REPOSITORY_H
