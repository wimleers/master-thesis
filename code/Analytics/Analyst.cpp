#include "Analyst.h"

namespace Analytics {

    Analyst::Analyst(float minSupport, float minConfidence) {
        this->minSupport = minSupport;
        this->minConfidence = minConfidence;
    }


    void Analyst::addRuleConsequentRequirement(ItemName item) {
        // If an item is required to be in the rule consequent, it evidently
        // must also be in the frequent itemsets. Therefore, also add this
        // item to the filters.
        this->addFilter(item);

        if (!this->ruleConsequentRequirements.contains(item))
            this->ruleConsequentRequirements.append(item);
    }


    /**
     * Add a filter. When association rule mining is performed, at least one
     * of these filters will have to be applicable for a rule to qualify
     * (i.e. the items in the transaction must match *any* of these filters).
     *
     * @param item
     *   An item to filter on.
     */
    void Analyst::addFilter(ItemName item) {
        if (!this->filterItems.contains(item))
            this->filterItems.append(item);
    }


    //------------------------------------------------------------------------
    // Protected slots.

    void Analyst::analyzeTransactions(const QList<QStringList> &transactions) {
        this->performMining(transactions);
    }


    //------------------------------------------------------------------------
    // Protected methods.

    void Analyst::performMining(const QList<QStringList> & transactions) {
//        QTime timer;
        qDebug() << "starting mining";
        FPGrowth * fpgrowth = new FPGrowth(transactions, this->minSupport);
        fpgrowth->setTransactionRequirements(this->filterItems);
        QList<ItemList> frequentItemsets = fpgrowth->mineFrequentItemsets();
        qDebug() << "frequent itemset mining complete";

        ItemList requirements;
        foreach (ItemName name, this->ruleConsequentRequirements) {
#ifdef DEBUG
            requirements.append(Item(fpgrowth->getItemID(name), fpgrowth->getItemIDNameHash()));
#else
            requirements.append(Item(fpgrowth->getItemID(name)));
#endif
        }

        QList<AssociationRule> associationRules = RuleMiner::mineAssociationRules(frequentItemsets, this->minConfidence, requirements);
        qDebug() << "mining association rules complete";

        qDebug() << associationRules;

        delete fpgrowth;
    }

}
