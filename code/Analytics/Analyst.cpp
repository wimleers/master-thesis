#include "Analyst.h"

namespace Analytics {

    Analyst::Analyst(double minSupport, double maxSupportError, double minConfidence) {
        this->minSupport      = minSupport;
        this->maxSupportError = maxSupportError;
        this->minConfidence   = minConfidence;

        // Stats for the UI.
        this->allBatchesNumPageViews = 0;
        this->allBatchesNumTransactions = 0;
        this->allBatchesStartTime = 0;

        // Browsable concept hierarchy.
        this->conceptHierarchyModel = new QStandardItemModel(this);

#ifdef DEBUG
        this->frequentItemsetItemConstraints.itemIDNameHash = &this->itemIDNameHash;
        this->ruleConsequentItemConstraints.itemIDNameHash = &this->itemIDNameHash;
#endif

        this->fpstream = new FPStream(this->minSupport, this->maxSupportError, &this->itemIDNameHash, &this->itemNameIDHash, &this->sortedFrequentItemIDs);
        connect(this->fpstream, SIGNAL(batchProcessed()), this, SLOT(fpstreamProcessedBatch()));
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

    /**
     * Extract the episode from an itemset and convert all item IDs to item
     * names. Essential for the UI.
     *
     * @param itemset
     *   The itemset (of item IDs) from which to extract the episode.
     * @return
     *   A pair: the first parameter is the episode item, the second parameter
     *   is a list that contains the remaining episode names.
     */
    QPair<ItemName, ItemNameList> Analyst::extractEpisodeFromItemset(ItemIDList itemset) const {
        ItemNameList itemNames;
        ItemName episodeName;

        // First, map to item names.
        foreach (ItemID id, itemset)
            itemNames.append(this->itemIDNameHash[id]);

        // Next, filter out the episode item.
        foreach (ItemName name, itemNames) {
            if (name.startsWith("episode")) {
                episodeName = name;
                itemNames.removeAll(name);
                break;
            }
        }

        return qMakePair(episodeName, itemNames);
    }


    //------------------------------------------------------------------------
    // Public slots.

    void Analyst::analyzeTransactions(const QList<QStringList> &transactions, double transactionsPerEvent, Time start, Time end) {
        // Stats for the UI.
        this->currentBatchStartTime = start;
        this->currentBatchEndTime = end;
        this->currentBatchNumPageViews = transactions.size() / transactionsPerEvent;
        this->currentBatchNumTransactions = transactions.size();
        this->timer.start();

        // Necessary to be able to update the browsable concept hierarchy in
        // Analyst::fpstreamProcessedBatch().
        this->uniqueItemsBeforeMining = this->itemIDNameHash.size();

        // Notify the UI.
        emit analyzing(true, this->currentBatchStartTime, this->currentBatchEndTime, this->currentBatchNumPageViews, this->currentBatchNumTransactions);

        // Perform the actual mining.
        this->performMining(transactions, transactionsPerEvent);

        // Since the mining above is performed asynchronously, this is NOT the
        // place where we know the calculations end. Only FP-Stream can know,
        // hence FP-Stream's processedBatch() signal is the correct indicator.
        // This signal is then sent from the @function fpstreamProcessedBatch()
        // slot.
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
        // Notify the UI.
        emit mining(true);

        this->timer.start();

        // First, consider each item for use with constraints.
        this->frequentItemsetItemConstraints.preprocessItemIDNameHash(this->itemIDNameHash);
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

        int duration = this->timer.elapsed();

        emit minedRules(from, to, associationRules, this->fpstream->getNumEventsInRange(from, to));

        // Notify the UI.
        emit mining(false);
        emit minedDuration(duration);
    }

    void Analyst::mineAndCompareRules(uint fromOlder, uint toOlder, uint fromNewer, uint toNewer) {
        // Notify the UI.
        emit mining(true);

        this->timer.start();

        // First, consider each item for use with constraints.
        this->frequentItemsetItemConstraints.preprocessItemIDNameHash(this->itemIDNameHash);
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
        // Calculate the number of events in the intersected range. The two time
        // ranges may either overlap (i.e. have an intersection) or not (i.e. be
        // disjoint).
        // e.g.:
        //   * 2-4, 5-8 or 5-8, 2-4 -> disjoint
        //   * 2-5, 4-8 or 4-8, 2-5 -> intersection
        bool olderTimeRangeIsActuallyOlder = (fromOlder <= fromNewer);
        uint actualOlderFrom = (olderTimeRangeIsActuallyOlder) ? fromOlder : fromNewer;
        uint actualOlderTo   = (olderTimeRangeIsActuallyOlder) ? toOlder : toNewer;
        uint actualNewerFrom = (olderTimeRangeIsActuallyOlder) ? fromNewer : fromOlder;
        uint actualNewerTo   = (olderTimeRangeIsActuallyOlder) ? toNewer : toOlder;
        SupportCount supportForIntersectedRange;
        if (actualOlderTo > actualNewerFrom) // Overlap
            supportForIntersectedRange = eventsPerBatch->getSupportForRange(actualOlderFrom, actualNewerTo);
        else
            supportForIntersectedRange = supportForOlderRange + supportForNewerRange;

        QList<AssociationRule> comparedRules;
        QList<Confidence> confidenceVariance;
        QList<float> supportVariance;

        // Intersected rules.
        QList<AssociationRule> intersectedRules = newerRules.toSet().intersect(olderRules.toSet()).toList();
        comparedRules.append(intersectedRules);
        int n, o;
        foreach (const AssociationRule & rule, intersectedRules) {
            n = newerRules.indexOf(rule);
            o = olderRules.indexOf(rule);
            confidenceVariance.append(newerRules[n].confidence - olderRules[o].confidence);
            supportVariance.append((1.0 * newerRules[n].support / supportForNewerRange) - (1.0 * olderRules[o].support / supportForOlderRange));
        }

        // Newer-only rules.
        QList<AssociationRule> newerOnlyRules = newerRules.toSet().subtract(intersectedRules.toSet()).toList();
        comparedRules.append(newerOnlyRules);
        for (int i = 0; i < newerOnlyRules.size(); i++) {
            confidenceVariance.append(1.0);
            supportVariance.append(1.0);
        }

        // Older-only rules.
        QList<AssociationRule> olderOnlyRules = olderRules.toSet().subtract(intersectedRules.toSet()).toList();
        comparedRules.append(olderOnlyRules);
        for (int i = 0; i < olderOnlyRules.size(); i++) {
            confidenceVariance.append(-1.0);
            supportVariance.append(-1.0);
        }

        int duration = this->timer.elapsed();

        emit comparedMinedRules(fromOlder, toOlder,
                                fromNewer, toNewer,
                                intersectedRules,
                                olderRules,
                                newerRules,
                                comparedRules,
                                confidenceVariance,
                                supportVariance,
                                supportForIntersectedRange,
                                supportForNewerRange,
                                supportForOlderRange);

        // Notify the UI.
        emit mining(false);
        emit minedDuration(duration);
    }


    //------------------------------------------------------------------------
    // Protected slots.

    void Analyst::fpstreamProcessedBatch() {
        int duration = this->timer.elapsed();
        if (this->allBatchesStartTime == 0)
            this->allBatchesStartTime = this->currentBatchStartTime;
        this->allBatchesNumPageViews += this->currentBatchNumPageViews;
        this->allBatchesNumTransactions += this->currentBatchNumTransactions;
        this->currentBatchNumPageViews = 0;
        this->currentBatchNumTransactions = 0;

        // Update the browsable concept hierarchy.
        this->updateConceptHierarchyModel(this->uniqueItemsBeforeMining);

        emit processedBatch();
        emit analyzing(false, 0, 0, 0, 0);
        emit analyzedDuration(duration);
        emit stats(
                    this->allBatchesStartTime,
                    this->currentBatchEndTime,
                    this->allBatchesNumPageViews,
                    this->allBatchesNumTransactions,
                    this->itemIDNameHash.size(),
                    this->fpstream->getNumFrequentItems(),
                    this->fpstream->getPatternTreeSize()
        );
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

    void Analyst::updateConceptHierarchyModel(int itemsAlreadyProcessed) {
        if (this->itemIDNameHash.size() <= itemsAlreadyProcessed)
            return;

        ItemName item, parent, child;
        QStringList parts;
        for (int id = itemsAlreadyProcessed; id < this->itemIDNameHash.size(); id++) {
            item = this->itemIDNameHash[(ItemID) id];
            parts = item.split(':', QString::SkipEmptyParts);

            // Ban "duration:*" from the concept hierarchy, since we only accept
            // "duration:slow" in the first place.
            if (parts[0].compare("duration") == 0)
                continue;

            // Ban "url:*" from the concept hierarchy, as it is fairly useless.
            if (parts[0].compare("url") == 0)
                continue;

            // Update the concept hierarchy model.
            parent = parts[0];
            // Root level.
            if (!this->conceptHierarchyHash.contains(parent)) {
                QStandardItem * modelItem = new QStandardItem(parent);
                modelItem->setData(parent.toUpper(), Qt::UserRole); // For sorting.

                // Store in hierarchy.
                this->conceptHierarchyHash.insert(parent, modelItem);
                QStandardItem * root = this->conceptHierarchyModel->invisibleRootItem();
                root->appendRow(modelItem);
            }
            // Subsequent levels.
            for (int p = 1; p < parts.size(); p++) {
                child = parent + ':' + parts[p];
                if (!this->conceptHierarchyHash.contains(child)) {
                    QStandardItem * modelItem = new QStandardItem(parts[p]);
                    modelItem->setData(parts[p].toUpper(), Qt::UserRole); // For sorting.

                    // Store in hierarchy.
                    this->conceptHierarchyHash.insert(child, modelItem);
                    QStandardItem * parentModelItem = this->conceptHierarchyHash[parent];
                    parentModelItem->appendRow(modelItem);
                }
                parent = child;
            }
        }

        // Sort the model case-insensitively.
        this->conceptHierarchyModel->setSortRole(Qt::UserRole);
        this->conceptHierarchyModel->sort(0, Qt::AscendingOrder);
    }
}
