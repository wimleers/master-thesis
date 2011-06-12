#include "Analyst.h"

namespace Analytics {

    Analyst::Analyst(double minSupport, double maxSupportError, double minConfidence) {
        this->minSupport      = minSupport;
        this->maxSupportError = maxSupportError;
        this->minConfidence   = minConfidence;

        this->fpstream = new FPStream(this->minSupport, this->maxSupportError, &this->itemIDNameHash, &this->itemNameIDHash, &this->sortedFrequentItemIDs);
    }

    Analyst::~Analyst() {
        delete this->fpstream;
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
     * Add a rule consequent item constraint of a given constraint type. When
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

    /**
     * Override of QObject::moveToThread(), to automatically also move the
     * FPStream object to the other thread.
     */
    void Analyst::moveToThread(QThread * thread) {
        QObject::moveToThread(thread);
        this->fpstream->moveToThread(thread);
    }


    //------------------------------------------------------------------------
    // Public slots.

    void Analyst::analyzeTransactions(const QList<QStringList> &transactions, double transactionsPerEvent) {
        this->performMining(transactions, transactionsPerEvent);
    }

    /**
     * Mine rules over a range of buckets (i.e., a range of time).
     *
     * @param from
     *   The range starts at this bucket.
     * @param to
     *   The range ends at this bucket.
     */
    void Analyst::mineRules(uint from, uint to) {
        // First, consider each item for use with constraints.
        this->ruleConsequentItemConstraints.preprocessItemIDNameHash(this->itemIDNameHash);

        // Now, mine for association rules.
        QList<AssociationRule> associationRules = RuleMiner::mineAssociationRules(
                this->fpstream->getPatternTree().getFrequentItemsetsForRange(
                        this->fpstream->calculateMinSupportForRange(from, to),
                        this->frequentItemsetItemConstraints,
                        from,
                        to
                ),
                this->minConfidence,
                this->ruleConsequentItemConstraints,
                this->fpstream->getPatternTree(),
                from,
                to
        );

        qDebug() << "mining association rules complete, # association rules:" << associationRules.size();
        qDebug() << associationRules;

        emit minedRules(from, to, associationRules);
    }

    void Analyst::mineAndCompareRules(uint fromOlder, uint toOlder, uint fromNewer, uint toNewer) {
        // First, consider each item for use with constraints.
        this->ruleConsequentItemConstraints.preprocessItemIDNameHash(this->itemIDNameHash);

        // Now, mine the association rules for the "older" range.
        QList<AssociationRule> olderRules = RuleMiner::mineAssociationRules(
                this->fpstream->getPatternTree().getFrequentItemsetsForRange(
                        this->fpstream->calculateMinSupportForRange(fromOlder, toOlder),
                        this->frequentItemsetItemConstraints,
                        fromOlder,
                        toOlder
                ),
                this->minConfidence,
                this->ruleConsequentItemConstraints,
                this->fpstream->getPatternTree(),
                fromOlder,
                toOlder
        );

        // Now, mine the association rules for the "newer" range.
        QList<AssociationRule> newerRules = RuleMiner::mineAssociationRules(
                this->fpstream->getPatternTree().getFrequentItemsetsForRange(
                        this->fpstream->calculateMinSupportForRange(fromNewer, toNewer),
                        this->frequentItemsetItemConstraints,
                        fromNewer,
                        toNewer
                ),
                this->minConfidence,
                this->ruleConsequentItemConstraints,
                this->fpstream->getPatternTree(),
                fromNewer,
                toNewer
        );

        // Finally, compare the rules for the "older" and "newer" range.
        TiltedTimeWindow const * const eventsPerBatch = this->fpstream->getEventsPerBatch();
        SupportCount supportForNewerRange = eventsPerBatch->getSupportForRange(fromNewer, toNewer);
        SupportCount supportForOlderRange = eventsPerBatch->getSupportForRange(fromOlder, toOlder);

        QList<AssociationRule> intersectedRules = newerRules.toSet().intersect(olderRules.toSet()).toList();
        QList<Confidence> confidenceVariance;
        QList<float> supportVariance;
        int n, o;
        foreach (AssociationRule rule, intersectedRules) {
            n = newerRules.indexOf(rule);
            o = olderRules.indexOf(rule);
            confidenceVariance.append(newerRules[n].confidence - olderRules[o].confidence);
            supportVariance.append((1.0 * newerRules[n].support / supportForNewerRange) - (1.0 * olderRules[o].support / supportForOlderRange));
        }

        qDebug() << olderRules.size() << newerRules.size() << intersectedRules.size() << supportVariance;
        emit comparedMinedRules(fromOlder, toOlder,
                                fromNewer, toNewer,
                                olderRules,
                                newerRules,
                                intersectedRules,
                                confidenceVariance,
                                supportVariance);
    }


    //------------------------------------------------------------------------
    // Protected methods.

    void Analyst::performMining(const QList<QStringList> & transactions, double transactionsPerEvent) {
        bool fpstream = true;

        if (!fpstream) {
//            qDebug() << "----------------------> FPGROWTH";
        // Clear these every time, to ensure the original behavior.
        this->itemIDNameHash.clear();
        this->itemNameIDHash.clear();
        this->sortedFrequentItemIDs.clear();

        qDebug() << "starting mining, # transactions: " << transactions.size();
        FPGrowth * fpgrowth = new FPGrowth(transactions, ceil(this->minSupport * transactions.size() / transactionsPerEvent), &this->itemIDNameHash, &this->itemNameIDHash, &this->sortedFrequentItemIDs);
        fpgrowth->setConstraints(this->frequentItemsetItemConstraints);
        fpgrowth->setConstraintsForRuleConsequents(this->ruleConsequentItemConstraints);
        QList<FrequentItemset> frequentItemsets = fpgrowth->mineFrequentItemsets(false);
        qDebug() << "frequent itemset mining complete, # frequent itemsets:" << frequentItemsets.size();

        /*
        this->ruleConsequentItemConstraints = fpgrowth->getPreprocessedConstraints();
        QList<AssociationRule> associationRules = RuleMiner::mineAssociationRules(frequentItemsets, this->minConfidence, this->ruleConsequentItemConstraints, fpgrowth);
        qDebug() << "mining association rules complete, # association rules:" << associationRules.size();

        qDebug() << associationRules;
        */

        delete fpgrowth;

        } else {
//            qDebug() << "----------------------> FPSTREAM";

        // Temporary show the FPStream datastructure (PatternTree) as it's
        // being built, until rule mining has also been implemented.
        static bool initial = true;
        if (initial) {
            this->fpstream->setConstraints(this->frequentItemsetItemConstraints);
            this->fpstream->setConstraintsToPreprocess(this->ruleConsequentItemConstraints);
            initial = false;
        }
        this->fpstream->processBatchTransactions(transactions, transactionsPerEvent);
        /*
        qDebug() << this->fpstream->getPatternTree().getNodeCount();
        qDebug() << this->itemIDNameHash.size() << this->itemNameIDHash.size() << this->sortedFrequentItemIDs.size();
        qDebug() << this->fpstream->getPatternTree();
        */
        }
    }

}
