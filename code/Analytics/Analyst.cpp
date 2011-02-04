#include "Analyst.h"

namespace Analytics {

    Analyst::Analyst(float minSupport, float minConfidence) {
        this->minSupport = minSupport;
        this->minConfidence = minConfidence;
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
        fpgrowth->setFilterItems(this->filterItems);
        qDebug() << "set filter items complete";
        fpgrowth->preprocessingPhase1();
        qDebug() << "preprocessing phase 1 complete";
        fpgrowth->preprocessingPhase2();
        qDebug() << "preprocessing phase 2 complete";
        QList<ItemList> frequentItemsets = fpgrowth->calculatingPhase1();
        qDebug() << "calculating phase 1 complete";

        QList<AssociationRule> associationRules = RuleMiner::mineAssociationRules(frequentItemsets, this->minConfidence);
        qDebug() << "mining association rules complete";

        qDebug() << associationRules;

        delete fpgrowth;
        qDebug() << "deleted fpgrowth";

    }

}
