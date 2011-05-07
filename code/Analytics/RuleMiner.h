#ifndef RULEMINER_H
#define RULEMINER_H

#include "Item.h"
#include "Constraints.h"
#include "FPGrowth.h"
#include <QList>


namespace Analytics {

#ifdef DEBUG
    //#define RULEMINER_DEBUG 0
#endif

    class RuleMiner {
    public:
        static QList<AssociationRule> mineAssociationRules(QList<FrequentItemset> frequentItemsets, float minimumConfidence, const Constraints & ruleConsequentConstraints, const FPGrowth * fpgrowth);

    protected:
        static QList<AssociationRule> generateAssociationRulesForFrequentItemset(FrequentItemset frequentItemset, QList<ItemIDList> consequents, float minimumConfidence, const FPGrowth * fpgrowth);
        static ItemIDList getAntecedent(const ItemIDList & frequentItemset, const ItemIDList & consequent);
        static QList<ItemIDList> generateCandidateItemsets(const QList<ItemIDList> & frequentItemsubsets);
    };

}
#endif // RULEMINER_H
