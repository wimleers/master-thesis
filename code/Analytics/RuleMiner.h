#ifndef RULEMINER_H
#define RULEMINER_H

#include "Item.h"
#include "Constraints.h"
#include "FPGrowth.h"
#include "PatternTree.h"
#include <QList>


namespace Analytics {

#ifdef DEBUG
    //#define RULEMINER_DEBUG 0
#endif

    class RuleMiner {
    public:
        static QList<AssociationRule> mineAssociationRules(QList<FrequentItemset> frequentItemsets, Confidence minimumConfidence, const Constraints & ruleConsequentConstraints, const FPGrowth * fpgrowth);
        static QList<AssociationRule> mineAssociationRules(QList<FrequentItemset> frequentItemsets, Confidence minimumConfidence, const Constraints & ruleConsequentConstraints, const PatternTree & patternTree, uint from, uint to);

    protected:
        static QList<AssociationRule> generateAssociationRulesForFrequentItemset(FrequentItemset frequentItemset, QList<ItemIDList> consequents, Confidence minimumConfidence, const FPGrowth * fpgrowth);
        static QList<AssociationRule> generateAssociationRulesForFrequentItemset(FrequentItemset frequentItemset, QList<ItemIDList> consequents, Confidence minimumConfidence, const PatternTree & patternTree, uint from, uint to);
        static ItemIDList getAntecedent(const ItemIDList & frequentItemset, const ItemIDList & consequent);
        static QList<ItemIDList> generateCandidateItemsets(const QList<ItemIDList> & frequentItemsubsets);
    };

}
#endif // RULEMINER_H
