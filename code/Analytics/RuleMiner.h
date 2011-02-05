#ifndef RULEMINER_H
#define RULEMINER_H

#include "Item.h"
#include "FPGrowth.h"
#include <QList>


namespace Analytics {

#ifdef DEBUG
    //#define RULEMINER_DEBUG 0
#endif

    class RuleMiner {
    public:
        static QList<AssociationRule> mineAssociationRules(QList<ItemList> frequentItemsets, float minimumConfidence, ItemList ruleConsequentRequirements);

    protected:
        static QList<SupportCount> calculateSupportCountsForFrequentItemsets(QList<ItemList> frequentItemsets);
        static SupportCount calculateSupportCountForFrequentItemset(ItemList frequentItemset);
        static QList<AssociationRule> generateAssociationRulesForFrequentItemset(ItemList frequentItemset, QList<ItemList> consequents, QList<ItemList> frequentItemsets, QList<SupportCount> frequentItemsetsSupportCounts, float minimumConfidence);
        static ItemList getAntecedent(ItemList frequentItemset, ItemList consequent);
        static QList<ItemList> generateCandidateItemsets(QList<ItemList> frequentItemsubsets);
    };

}
#endif // RULEMINER_H