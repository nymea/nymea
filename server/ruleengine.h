#ifndef RULEENGINE_H
#define RULEENGINE_H

#include "rule.h"
#include "event.h"

#include <QObject>
#include <QList>
#include <QUuid>

class RuleEngine : public QObject
{
    Q_OBJECT
public:
    enum RuleError {
        RuleErrorNoError,
        RuleErrorRuleNotFound,
        RuleErrorDeviceNotFound,
        RuleErrorEventTypeNotFound
    };

    explicit RuleEngine(QObject *parent = 0);

    QList<Action> evaluateEvent(const Event &event);

    RuleError addRule(const Event &event, const QList<Action> &actions);
    RuleError addRule(const Event &event, const QList<State> &states, const QList<Action> &actions);
    QList<Rule> rules() const;

    RuleError removeRule(const QUuid &ruleId);

signals:
    void ruleAdded(const QUuid &ruleId);
    void ruleRemoved(const QUuid &ruleId);

private:
    QString m_settingsFile;
    QList<Rule> m_rules;

};

#endif // RULEENGINE_H
