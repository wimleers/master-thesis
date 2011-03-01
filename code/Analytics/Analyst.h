#ifndef ANALYTICS_H
#define ANALYTICS_H

#include <QObject>
#include <QTime>
#include <QList>
#include <QHash>
#include <QStringList>

#include "Item.h"
#include "Constraints.h"
#include "FPGrowth.h"
#include "RuleMiner.h"
#include "FPNode.h"


namespace Analytics {

    class Analyst : public QObject {
        Q_OBJECT

    public:
        Analyst(float minSupport, float minConfidence);
        void addFrequentItemsetItemConstraint(ItemName item, ItemConstraintType type);
        void addRuleConsequentItemConstraint(ItemName item, ItemConstraintType type);

    protected slots:
        void analyzeTransactions(const QList<QStringList> & transactions);

    protected:
        void performMining(const QList<QStringList> & transactions);

        float minSupport;
        float minConfidence;

        Constraints frequentItemsetItemConstraints;
        Constraints ruleConsequentItemConstraints;
    };
}

#endif // ANALYST_H
