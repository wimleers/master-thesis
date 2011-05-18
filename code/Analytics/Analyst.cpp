#include "Analyst.h"

namespace Analytics {

    Analyst::Analyst(double minSupport, double maxSupportError, double minConfidence) {
        this->minSupport      = minSupport;
        this->maxSupportError = maxSupportError;
        this->minConfidence   = minConfidence;

        this->fpstream = new FPStream(this->minSupport, this->maxSupportError, &this->itemIDNameHash, &this->itemNameIDHash, &this->sortedFrequentItemIDs);
    }

    /**
     * Add a frequent itemset item constraint of a given constraint type. When
     * frequent itemsets are being generated, only those will be considered
     * that match the constraints defined here.
     *
     * @param item
     *   An item name.
     * @param type
     *   The constraint type.
     */
    void Analyst::addFrequentItemsetItemConstraint(ItemName item, ItemConstraintType type) {
        this->frequentItemsetItemConstraints.addItemConstraint(item, type);
    }

    /**
     * Add a rule consequentitem constraint of a given constraint type. When
     * rules are being mined, only those will be considered that match the
     * constraints defined here.
     *
     * @param item
     *   An item name.
     * @param type
     *   The constraint type.
     */
    void Analyst::addRuleConsequentItemConstraint(ItemName item, ItemConstraintType type) {
        // If an item is required to be in the rule consequent, it evidently
        // must also be in the frequent itemsets. Therefore, the same item
        // constraints that apply to the rule consequents also apply to
        // frequent itemsets.
        // By also applying these item constraints to frequent itemset
        // generation, we reduce the amount of work to be done to a minimum.
        this->frequentItemsetItemConstraints.addItemConstraint(item, type);

        this->ruleConsequentItemConstraints.addItemConstraint(item, type);
    }


    //------------------------------------------------------------------------
    // Protected slots.

    void Analyst::analyzeTransactions(const QList<QStringList> &transactions) {
        this->performMining(transactions);
    }


    //------------------------------------------------------------------------
    // Protected methods.

    void Analyst::performMining(const QList<QStringList> & transactions) {
        // Clear these every time, to ensure the original behavior.
        this->itemIDNameHash.clear();
        this->itemNameIDHash.clear();
        this->sortedFrequentItemIDs.clear();

        qDebug() << "starting mining, # transactions: " << transactions.size();
        FPGrowth * fpgrowth = new FPGrowth(transactions, ceil(this->minSupport * 4000), &this->itemIDNameHash, &this->itemNameIDHash, &this->sortedFrequentItemIDs);
        fpgrowth->setConstraints(this->frequentItemsetItemConstraints);
        fpgrowth->setConstraintsToPreprocess(this->ruleConsequentItemConstraints);
        QList<FrequentItemset> frequentItemsets = fpgrowth->mineFrequentItemsets();
        qDebug() << "frequent itemset mining complete, # frequent itemsets:" << frequentItemsets.size();

        this->ruleConsequentItemConstraints = fpgrowth->getPreprocessedConstraints();
        QList<AssociationRule> associationRules = RuleMiner::mineAssociationRules(frequentItemsets, this->minConfidence, this->ruleConsequentItemConstraints, fpgrowth);
        qDebug() << "mining association rules complete, # association rules:" << associationRules.size();

        qDebug() << associationRules;

        delete fpgrowth;
    }

}
