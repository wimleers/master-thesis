#ifndef ANALYTICS_H
#define ANALYTICS_H

#include <QObject>
#include <QTime>
#include <QList>
#include <QStringList>

#include "Item.h"
#include "FPGrowth.h"
#include "RuleMiner.h"
#include "FPNode.h"


namespace Analytics {

    class Analyst : public QObject {
        Q_OBJECT

    public:
        Analyst(float minSupport, float minConfidence);
        void addRuleConsequentRequirement(ItemName item);
        void addFilter(ItemName item);

    protected slots:
        void analyzeTransactions(const QList<QStringList> & transactions);

    protected:
        void performMining(const QList<QStringList> & transactions);

        QList<ItemName> filterItems;
        QList<ItemName> ruleConsequentRequirements;
        float minSupport;
        float minConfidence;
    };
}

#endif // ANALYST_H
