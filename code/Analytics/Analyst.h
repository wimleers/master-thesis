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
#include "FPStream.h"
#include "RuleMiner.h"


namespace Analytics {

    class Analyst : public QObject {
        Q_OBJECT

    public:
        Analyst(double minSupport, double maxSupportError, double minConfidence);
        void addFrequentItemsetItemConstraint(ItemName item, ItemConstraintType type);
        void addRuleConsequentItemConstraint(ItemName item, ItemConstraintType type);

    signals:
        void minedRules(uint from, uint to, QList<AssociationRule> associationRules);
        void comparedMinedRules(uint fromOlder, uint toOlder,
                                uint fromNewer, uint toNewer,
                                QList<AssociationRule> olderRules,
                                QList<AssociationRule> newerRules,
                                QList<AssociationRule> intersectedRules,
                                QList<Confidence> confidenceVariance,
                                QList<float> supportVariance);

    public slots:
        void analyzeTransactions(const QList<QStringList> & transactions, double transactionsPerEvent);
        void mineRules(uint from, uint to);
        void mineAndCompareRules(uint fromOlder, uint toOlder, uint fromNewer, uint toNewer);

    protected:
        void performMining(const QList<QStringList> & transactions, double transactionsPerEvent);

        FPStream * fpstream;
        double minSupport;
        double maxSupportError;
        double minConfidence;

        Constraints frequentItemsetItemConstraints;
        Constraints ruleConsequentItemConstraints;

        ItemIDNameHash itemIDNameHash;
        ItemNameIDHash itemNameIDHash;
        ItemIDList sortedFrequentItemIDs;
    };
}

#endif // ANALYST_H
