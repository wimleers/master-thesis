#include "FPGrowth.h"

namespace Analytics {

    FPGrowth::FPGrowth(const QList<QStringList> & transactions, SupportCount minSupportAbsolute, ItemIDNameHash * itemIDNameHash, ItemNameIDHash * itemNameIDHash, ItemIDList * sortedFrequentItemIDs) {
        this->itemIDNameHash        = itemIDNameHash;
        this->itemNameIDHash        = itemNameIDHash;
        this->sortedFrequentItemIDs = sortedFrequentItemIDs;

        this->transactions = transactions;

        this->minSupportAbsolute = minSupportAbsolute;

        this->tree = new FPTree();
#ifdef DEBUG
        this->tree->itemIDNameHash = this->itemIDNameHash;
#endif
    }

    FPGrowth::~FPGrowth() {
        delete this->tree;
    }

    /**
     * Mine frequent itemsets. (First scan the transactions, then build the
     * FP-tree, then generate the frequent itemsets from there.)
     *
     * @param asynchronous
     *   See the explanation for the identically named parameter of @fn
     *   generateFrequentItemsets().
     * @return
     *   The frequent itemsets that were found.
     */
    QList<FrequentItemset> FPGrowth::mineFrequentItemsets(bool asynchronous) {
        this->scanTransactions();
        this->buildFPTree();
        return this->generateFrequentItemsets(this->tree, FrequentItemset(), asynchronous);
    }

    /**
     * Calculate the upper bound of the support count for an itemset.
     *
     * @param itemset
     *   The itemset to calculate the upper bound of its support count for.
     * @return
     *   The upper bound of the support count for this itemset.
     */
    SupportCount FPGrowth::calculateSupportCountUpperBound(const ItemIDList & itemset) const {
        // For itemsets of size 1, we can simply use the QHash that contains
        // all frequent items' support counts, since it contains the exact
        // data we need (this is FPGrowth::totalFrequentSupportCounts). Thus,
        // in this case, we don't really calculate the upper bound: we
        // immediately return the correct support count.
        // For larger itemsets, we'll have to calculate the upper bound by
        // exploiting some properties. See the code below for details.
        if (itemset.size() == 1) {
            return this->totalFrequentSupportCounts[itemset[0]];
        }
        else {
            ItemIDList optimizedItemset = this->optimizeItemset(itemset);

            // The last item is the one with the least support, since it is
            // optimized for this by FPGrowth::optimizeItemset.
            ItemID lastItemID = optimizedItemset[optimizedItemset.size() - 1];

            // Since all items in a frequent itemset must themselves also be
            // frequent, it must be available in the previously calculated
            // FPGrowth::totalFrequentSupportCounts. An itemset's support
            // count is only as frequent as its least supported item, which is
            // the last item thanks to the FPGrowth::optimizeTransaction()
            // call.
            // Hence the itemset's upper bound support count is equal to the
            // support count of the last item in the itemset.
            return this->totalFrequentSupportCounts[lastItemID];
        }
    }

    /**
     * Calculate the exact support count for an itemset.
     *
     * @param itemset
     *   The itemset to calculate the exact support count for.
     * @return
     *   The exact support count for this itemset
     */
    SupportCount FPGrowth::calculateSupportCountExactly(const ItemIDList & itemset) const {
        // For itemsets of size 1, we can simply use the QHash that contains
        // all frequent items' support counts, since it contains the exact
        // data we need (this is FPGrowth::totalFrequentSupportCounts).
        // For larger itemsets, we'll have to get the exact support count by
        // examining the FP-tree.
        if (itemset.size() == 1) {
            return this->totalFrequentSupportCounts[itemset[0]];
        }
        else {
            // First optimize the itemset so that the item with the least
            // support is the last item.
            ItemIDList optimizedItemset = this->optimizeItemset(itemset);

            // Starting with the last item in the itemset:
            // 1) calculate its prefix paths
            // 2) filter these prefix paths to remove the items along these
            //    paths that don't meet the minimum support
            // 3) build the corresponding conditional FP-tree
            // Repeat this until we've reached the second item in the itemset,
            // then we have the support count for this item set.
            int last = optimizedItemset.size() - 1;
            FPTree * cfptree;
            QList<ItemList> prefixPaths;
            for (int whichItem = last; whichItem > 0; whichItem--) {
                // Step 1: calculate prefix paths.
                if (whichItem == last)
                    prefixPaths = this->tree->calculatePrefixPaths(optimizedItemset[whichItem]);
                else {
                    prefixPaths = cfptree->calculatePrefixPaths(optimizedItemset[whichItem]);
                    delete cfptree;
                }
                // Step 2: filter.
                prefixPaths = FPGrowth::filterPrefixPaths(prefixPaths, this->minSupportAbsolute);
                // Step 3: build the conditional FP-tree.
                // Note that it is impossible to end with zero prefix paths
                // after filtering, since the itemset that is passed to this
                // function consists of frequent items.
                cfptree = new FPTree();
#ifdef DEBUG
                cfptree->itemIDNameHash = this->itemIDNameHash;
#endif
                cfptree->buildTreeFromPrefixPaths(prefixPaths);
            }

            // The conditional FP-tree for the second item in the itemset
            // contains the support count for the itemset that was passed into
            // this function.
            return cfptree->getItemSupport(itemset[0]);
        }
    }


    //------------------------------------------------------------------------------
    // Protected slots.

    /**
     * Slot that receives a Transaction, optimizes it and adds it to the tree.
     *
     * @param transaction
     *   The transaction to process.
     */
    void FPGrowth::processTransaction(const Transaction & transaction) {
        Transaction optimizedTransaction;
        optimizedTransaction = this->optimizeTransaction(transaction);

        // It's possible that the optimized transaction has become empty if
        // none of the items in the given transaction meet or exceed the
        // minimum support.
        if (optimizedTransaction.size() > 0)
            this->tree->addTransaction(optimizedTransaction);
    }


    //------------------------------------------------------------------------
    // Protected static methods.

    /**
     * Given an ItemID -> SupportCount hash, sort ItemIDs by decreasing
     * support count.
     *
     * @param itemSupportCounts
     *   A QHash of ItemID -> SupportCount pairs.
     * @param ignoreList
     *   A list of ItemIDs to ignore (i.e. not include in the result).
     * @return
     *   A QList of ItemIDs, sorted by decreasing support count.
     */
    ItemIDList FPGrowth::sortItemIDsByDecreasingSupportCount(const QHash<ItemID, SupportCount> & itemSupportCounts, const ItemIDList * const ignoreList){
        QHash<SupportCount, ItemID> itemIDsBySupportCount;
        QSet<SupportCount> supportCounts;
        SupportCount supportCount;

        foreach (ItemID itemID, itemSupportCounts.keys()) {
            if (ignoreList->contains(itemID))
                continue;

            supportCount = itemSupportCounts[itemID];

            // Fill itemsBySupport by using QHash::insertMulti(), which allows
            // for multiple values for the same key.
            itemIDsBySupportCount.insertMulti(supportCount, itemID);

            // Fill supportCounts. Since this is a set, each unique support
            // count will only be stored once.
            supportCounts.insert(supportCount);
        }

        // Sort supportCounts from smaller to greater. But first convert from
        // a QSet to a QList, because sets cannot have an order by definition.
        QList<SupportCount> sortedSupportCounts;
        sortedSupportCounts = supportCounts.toList();
//        qSort(sortedSupportCounts);
        qSort(sortedSupportCounts.begin(), sortedSupportCounts.end(), qGreater<SupportCount>());

        // Store all ItemIDs, sorted by support count. If multiple ItemIDs
        // have the same SupportCount, sort them from small to large.
        ItemIDList sortedItemIDs;
        foreach (SupportCount support, sortedSupportCounts) {
            ItemIDList itemIDs = itemIDsBySupportCount.values(support);
            qSort(itemIDs);
            sortedItemIDs.append(itemIDs);
        }

        return sortedItemIDs;
    }

    /**
     * Remove items from the prefix paths based on the support counts of the
     * items *within* the prefix paths.
     *
     * @param prefixPaths
     *   The prefix paths to filter.
     * @param minSupportAbsolute
     *   The minimum absolute support count that should be met.
     * @return
     *   The filtered prefix paths.
     */
    QList<ItemList> FPGrowth::filterPrefixPaths(const QList<ItemList> & prefixPaths, SupportCount minSupportAbsolute) {
        QHash<ItemID, SupportCount> prefixPathsSupportCounts = FPTree::calculateSupportCountsForPrefixPaths(prefixPaths);

        QList<ItemList> filteredPrefixPaths;
        ItemList filteredPrefixPath;
        foreach (ItemList prefixPath, prefixPaths) {
            foreach (Item item, prefixPath) {
                if (prefixPathsSupportCounts[item.id] >= minSupportAbsolute)
                    filteredPrefixPath.append(item);
            }
            if (filteredPrefixPath.size() > 0)
                filteredPrefixPaths.append(filteredPrefixPath);
            filteredPrefixPath.clear();
        }

        return filteredPrefixPaths;
    }


    //------------------------------------------------------------------------
    // Protected methods.

    /**
     * Preprocess the transactions:
     * 1) map the item names to item IDs, so we only have to store numeric IDs
     *    in the FP-tree and conditional FP-trees, instead of entire strings
     * 2) determine the support count of each item (happens simultaneously
     *    with step 1)
     * 3) discard infrequent items' support count
     * 4) sort the frequent items by decreasing support count
     *
     * Also, each time when a new item name is mapped to an item id, it is
     * processed for use in constraints as well.
     */
    void FPGrowth::scanTransactions() {
        // Consider items with item names that have been mapped to  item IDs
        // in previous executions of FPGrowth for use with constraints.
        if (!this->itemIDNameHash->isEmpty()) {
            foreach (ItemID itemID, this->itemIDNameHash->keys()) {
                ItemName itemName = this->itemIDNameHash->value(itemID);
                this->constraints.preprocessItem(itemName, itemID);
                this->constraintsToPreprocess.preprocessItem(itemName, itemID);
            }
        }

        // Map the item names to item IDs. Maintain two dictionaries: one for
        // each look-up direction (name -> id and id -> name).
        ItemID itemID;
        foreach (QStringList transaction, this->transactions) {
            foreach (QString itemName, transaction) {
                // Look up the itemID for this itemName, or create it.
                if (!this->itemNameIDHash->contains(itemName)) {
                    itemID = this->itemNameIDHash->size();
                    this->itemNameIDHash->insert(itemName, itemID);
                    this->itemIDNameHash->insert(itemID, itemName);
                    this->totalFrequentSupportCounts.insert(itemID, 0);

                    // Consider this item for use with constraints.
                    this->constraints.preprocessItem(itemName, itemID);
                    this->constraintsToPreprocess.preprocessItem(itemName, itemID);
                }
                else
                    itemID = this->itemNameIDHash->value(itemName);

                this->totalFrequentSupportCounts[itemID]++;
            }
        }

        // Discard infrequent items' SupportCount.
        foreach (itemID, this->totalFrequentSupportCounts.keys()) {
            if (this->totalFrequentSupportCounts[itemID] < this->minSupportAbsolute) {
                this->totalFrequentSupportCounts.remove(itemID);

                // Remove infrequent items' ids from the preprocessed
                // constraints.
                this->constraints.removeItem(itemID);
                this->constraintsToPreprocess.removeItem(itemID);
            }
        }

        // Sort the frequent items' item ids by decreasing support count.
        this->sortedFrequentItemIDs->append(FPGrowth::sortItemIDsByDecreasingSupportCount(this->totalFrequentSupportCounts, this->sortedFrequentItemIDs));

#ifdef FPGROWTH_DEBUG
        qDebug() << "order:";
        foreach (itemID, *(this->sortedFrequentItemIDs)) {
            if (this->totalFrequentSupportCounts.contains(itemID))
                qDebug() << this->totalFrequentSupportCounts[itemID] << " times: " << Item(itemID, this->itemIDNameHash);
        }
#endif
    }

    /**
     * Build the FP-tree, by using the results from scanTransactions().
     *
     * TODO: figure out if this can be done in one pass with scanTransactions.
     * This should be doable since we can use the generated itemIDs as they
     * are being created.
     */
    void FPGrowth::buildFPTree() {
        Transaction transaction;

        foreach (QStringList transactionAsStrings, this->transactions) {
            transaction.clear();
            foreach (ItemName name, transactionAsStrings) {
#ifdef DEBUG
                transaction << Item((ItemID) this->itemNameIDHash->value(name), this->itemIDNameHash);
#else
                transaction << Item((ItemID) this->itemNameIDHash->value(name);
#endif
            }

            // The transaction in QStringList form has been converted to
            // QList<Item> form. Now process the transaction in this form.
            this->processTransaction(transaction);
        }

#ifdef FPGROWTH_DEBUG
        qDebug() << "Parsed" << this->transactions.size() << "transactions.";
        qDebug() << *this->tree;
#endif
    }

    /**
     * Optimize a transaction.
     *
     * This is achieved by sorting the items by decreasing support count. To
     * do this as fast as possible, this->sortedFrequentItemIDs is reused.
     * Because this->itemIDsSortedByTotalSupportCount does not include
     * infrequent items, these are also automatically removed by this simple
     * routine.
     *
     * @param transaction
     *   A transaction.
     * @return
     *   The optimized transaction.
     */
    Transaction FPGrowth::optimizeTransaction(const Transaction & transaction) const {
        Transaction optimizedTransaction;
        Item item;

        foreach (ItemID itemID, *(this->sortedFrequentItemIDs)) {
            item.id = itemID;
            if (transaction.contains(item))
                optimizedTransaction.append(transaction.at(transaction.indexOf(item)));
        }

        return optimizedTransaction;
    }

    /**
     * Optimize an ItemIDList.
     *
     * @see FPGrowth::optimizeTransaction(), which optimizes a list of Items,
     * whereas this method optimized a list of ItemIDs.
     *
     * @param itemset
     *   A list of ItemIDs.
     * @return
     *   The optimized list of ItemIDs.
     */
    ItemIDList FPGrowth::optimizeItemset(const ItemIDList & itemset) const {
        ItemIDList optimizedItemset;

        foreach (ItemID itemID, *(this->sortedFrequentItemIDs)) {
            if (itemset.contains(itemID))
                optimizedItemset.append(itemID);
        }

        return optimizedItemset;
    }

    /**
     * Generate the frequent itemsets recursively
     *
     * @param ctree
     *   Initially the entire FP-tree, but in subsequent (recursive) calls,
     *   a conditional FP-tree.
     * @param suffix
     *   The current frequent itemset suffix. Empty in the initial call, but
     *   automatically filled by this function when it recurses.
     * @param asynchronous
     *   FPGROWTH_ASYNC or FPGROWTH_SYNC
     *   By default, this method runs in a non-blocking (asynchronous)
     *   manner: it emits a signal for every frequent itemset it finds at a
     *   level. It is then up to the recipient of this signal to call this
     *   method again (i.e. to cause recursion) if it wants supersets of the
     *   signaled frequent itemset to be mined.
     *   When passing FPGROWTH_SYNC to this parameter, this method will run
     *   in a blocking (synchronous) manner, and will thus not allow another
     *   object to decide if supersets should also be mined. It will collect
     *   all frequent itemsets that can be found and then return all of them
     *   at once.
     * @return
     *   The entire list of frequent itemsets. The SupportCount for each Item
     *   still needs to be cleaned: it should be set to the minimum of all
     *   Items in each frequent itemset.
     */
    QList<FrequentItemset> FPGrowth::generateFrequentItemsets(const FPTree * ctree, const FrequentItemset & suffix, bool asynchronous) {
        bool frequentItemsetMatchesConstraints;
        QList<FrequentItemset> frequentItemsets;

        // First determine the order for the items in this particular tree,
        // based on the list that contains *all* items in this data set,
        // sorted by support count.
        // We do this mostly for cosmetic reasons. However, it also implies
        // that we will generate frequent itemsets for which the first item is
        // the most frequent, the second item is the second most frequent, etc.
        // Note: cannot replaced with a call to FPGrowth::optimizeTransaction()
        // because that works over a QList<Item>, wheras we have to deal with
        // a QList<ItemID> here.
        ItemIDList orderedItemIDs;
        ItemIDList itemIDsInTree = ctree->getItemIDs();
        foreach (ItemID itemID, *(this->sortedFrequentItemIDs))
            if (itemIDsInTree.contains(itemID))
                orderedItemIDs.append(itemID);

        // We don't need the unsorted item IDs anymore.
        itemIDsInTree.clear();

        // Now iterate over each of the ordered suffix items and generate
        // candidate frequent itemsets!
        foreach (ItemID prefixItemID, orderedItemIDs) {
            // Only if this prefix item's support meets or exceeds the minimum
            // support, it will be added as a frequent itemset (appended with
            // the received suffix of course).
            SupportCount prefixItemSupport = ctree->getItemSupport(prefixItemID);
            if (prefixItemSupport >= this->minSupportAbsolute) {
                // The current suffix item, when prepended to the received
                // suffix, is the next frequent itemset.
                // Additionally, this new frequent itemset will become the
                // next recursion's suffix.
                FrequentItemset frequentItemset(prefixItemID, prefixItemSupport, suffix);
#ifdef DEBUG
                frequentItemset.IDNameHash = this->itemIDNameHash;
#endif

                // Only store the current frequent itemset if it matches the
                // constraints.
                frequentItemsetMatchesConstraints = this->constraints.matchItemset(frequentItemset.itemset);
                if (!asynchronous && frequentItemsetMatchesConstraints) {
                    frequentItemsets.append(frequentItemset);
#ifdef FPGROWTH_DEBUG
                qDebug() << "\t\t\t\t new frequent itemset:" << frequentItemset;
#endif
                }


                // Check if there are supersets to be mined.
                FPTree * cfptree = this->considerFrequentItemsupersets(ctree, frequentItemset.itemset);
                if (cfptree != NULL && !asynchronous) {
                    // Attempt to generate more frequent itemsets, with the
                    // current frequent itemset as the suffix.
                    frequentItemsets.append(this->generateFrequentItemsets(cfptree, frequentItemset, asynchronous));

                    // This will make sure every conditional FP-tree gets
                    // deleted, but *not* the original tree. This is exactly
                    // what we want, since the original tree will be deleted
                    // in the destructor.
                    // This is the synchronous case, i.e. the case where no
                    // signals are emitted.
                    delete cfptree;
                }

                if (asynchronous)
                    emit this->minedFrequentItemset(frequentItemset, frequentItemsetMatchesConstraints, cfptree);
            }
        }

        if (asynchronous) {
            // This will make sure every conditional FP-tree gets deleted,
            // but *not* the original tree. This is exactly what we want,
            // since the original tree will be deleted in the destructor.
            // This is the asynchronous case, i.e. the case where no signals
            // are emitted.
            if (ctree != this->tree)
                delete ctree;

            // Necessary to terminate the algorithm in the asynchronous case.
            emit this->branchCompleted(suffix.itemset);
        }

        return frequentItemsets;
    }

    FPTree * FPGrowth::considerFrequentItemsupersets(const FPTree * ctree, const ItemIDList & frequentItemset) {
        // Calculate the prefix paths for the current prefix item
        // (which is a prefix to the current suffix, but when
        // calculating prefix paths, it's actually considered the
        // leading item ID of the suffix, i.e. as if it were the
        // leading item ID of the future suffix, which is in fact the
        // frequent itemset that we've just found).
        QList<ItemList> prefixPaths = ctree->calculatePrefixPaths(frequentItemset[0]);

        // Remove items from the prefix paths based that no longer
        // have sufficient support.
        prefixPaths = FPGrowth::filterPrefixPaths(prefixPaths, this->minSupportAbsolute);

        // If the conditional FP-tree would not be able to match the
        // constraints (which we can now by looking at the current
        // frequent itemset and the prefix paths support counts), then
        // just don't bother generating it.
        // This is effectively pruning the search space for frequent
        // itemsets.
        QHash<ItemID, SupportCount> prefixPathsSupportCounts = FPTree::calculateSupportCountsForPrefixPaths(prefixPaths);
        if (!this->constraints.matchSearchSpace(frequentItemset, prefixPathsSupportCounts))
            return NULL;

        // If no prefix paths remain after filtering, we won't be able
        // to generate any further frequent item sets.
        if (prefixPaths.size() > 0) {
            // Build the conditional FP-tree for these prefix paths,
            // by creating a new FP-tree and pretending the prefix
            // paths are transactions.
            FPTree * cfptree = new FPTree();
#ifdef DEBUG
            cfptree->itemIDNameHash = this->itemIDNameHash;
#endif
            cfptree->buildTreeFromPrefixPaths(prefixPaths);
#ifdef FPGROWTH_DEBUG
            qDebug() << *cfptree;
#endif

            return cfptree;
        }
        else
            return NULL;
    }
}
