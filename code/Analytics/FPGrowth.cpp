#include "FPGrowth.h"

namespace Analytics {

    FPGrowth::FPGrowth(const QList<QStringList> & transactions, SupportCount minSupportAbsolute) {
        this->transactions = transactions;

        this->minSupportAbsolute = minSupportAbsolute;

        this->tree = new FPTree();
    }

    FPGrowth::~FPGrowth() {
        delete this->tree;
    }

    /**
     * Mine frequent itemsets. (First scan the transactions, then build the
     * FP-tree, then generate the frequent itemsets from there.)
     *
     * @return
     *   The frequent itemsets that were found.
     */
    QList<ItemList> FPGrowth::mineFrequentItemsets() {
        this->scanTransactions();
        this->buildFPTree();
        return this->generateFrequentItemsets(this->tree);
    }

    /**
     * Calculate the upper bound of the support count for an itemset.
     *
     * @param itemset
     *   The itemset to calculate the upper bound of its support count for.
     * @return
     *   The upper bound of the support count for this itemset.
     */
    SupportCount FPGrowth::calculateSupportCountUpperBound(const ItemList & itemset) const {
        // For itemsets of size 1, we can simply use the QHash that contains
        // all frequent items' support counts, since it contains the exact
        // data we need (this is FPGrowth::totalFrequentSupportCounts). Thus,
        // in this case, we don't really calculate the upper bound: we
        // immediately return the correct support count.
        // For larger itemsets, we'll have to calculate the upper bound by
        // exploiting some properties. See the code below for details.
        if (itemset.size() == 1) {
            return this->totalFrequentSupportCounts[itemset[0].id];
        }
        else {
            ItemList optimizedItemset = this->optimizeTransaction(itemset);

            // The last item is the one with the least support, since it is
            // optimized for this by FPGrowth::optimizeTransaction.
            Item lastItem = optimizedItemset[optimizedItemset.size() - 1];

            // Since all items in a frequent itemset must themselves also be
            // frequent, it must be available in the previously calculated
            // FPGrowth::totalFrequentSupportCounts. An itemset's support
            // count is only as frequent as its least supported item, which is
            // the last item thanks to the FPGrowth::optimizeTransaction()
            // call.
            // Hence the itemset's upper bound support count is equal to the
            // support count of the last item in the itemset.
            return this->totalFrequentSupportCounts[lastItem.id];
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
    SupportCount FPGrowth::calculateSupportCountExactly(const ItemList & itemset) const {
        // For itemsets of size 1, we can simply use the QHash that contains
        // all frequent items' support counts, since it contains the exact
        // data we need (this is FPGrowth::totalFrequentSupportCounts).
        // For larger itemsets, we'll have to get the exact support count by
        // examining the FP-tree.
        if (itemset.size() == 1) {
            return this->totalFrequentSupportCounts[itemset[0].id];
        }
        else {
            // First optimize the itemset so that the item with the least
            // support is the last item.
            ItemList optimizedItemset = this->optimizeTransaction(itemset);

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
                    prefixPaths = this->tree->calculatePrefixPaths(optimizedItemset[whichItem].id);
                else {
                    prefixPaths = cfptree->calculatePrefixPaths(optimizedItemset[whichItem].id);
                    delete cfptree;
                }
                // Step 2: filter.
                prefixPaths = FPGrowth::filterPrefixPaths(prefixPaths, this->minSupportAbsolute);
                // Step 3: build the conditional FP-tree.
                // Note that it is impossible to end with zero prefix paths
                // after filtering, since the itemset that is passed to this
                // function consists of frequent items.
                cfptree = new FPTree(prefixPaths);
            }

            // The conditional FP-tree for the second item in the itemset
            // contains the support count for the itemset that was passed into
            // this function.
            return cfptree->getItemSupport(itemset[0].id);
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
     * Given an ItemID -> SupportCount hash, find sort ItemIDs by decreasing
     * support count.
     *
     * @param itemSupportCounts
     *   A QHash of ItemID -> SupportCount pairs.
     * @return
     *   A QList of ItemIDs, sorted by decreasing support count.
     */
    QList<ItemID> FPGrowth::sortItemIDsByDecreasingSupportCount(const QHash<ItemID, SupportCount> & itemSupportCounts){
        QHash<SupportCount, ItemID> itemIDsBySupportCount;
        QSet<SupportCount> supportCounts;
        SupportCount supportCount;

        foreach (ItemID itemID, itemSupportCounts.keys()) {
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
        QList<ItemID> sortedItemIDs;
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
        // Map the item names to item IDs. Maintain two dictionaries: one for
        // each look-up direction (name -> id and id -> name).
        ItemID itemID;
        foreach (QStringList transaction, this->transactions) {
            foreach (QString itemName, transaction) {
                // Look up the itemID for this itemName, or create it.
                if (!this->itemNameIDHash.contains(itemName)) {
                    itemID = this->itemNameIDHash.size();
                    this->itemNameIDHash.insert(itemName, itemID);
                    this->itemIDNameHash.insert(itemID, itemName);
                    this->totalFrequentSupportCounts.insert(itemID, 0);

                    // Consider this item for use with constraints.
                    this->constraints.preprocessItem(itemName, itemID);
                    this->constraintsToPreprocess.preprocessItem(itemName, itemID);
                }
                else
                    itemID = this->itemNameIDHash[itemName];

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
        this->frequentItemIDsSortedByTotalSupportCount = FPGrowth::sortItemIDsByDecreasingSupportCount(this->totalFrequentSupportCounts);

#ifdef FPGROWTH_DEBUG
        qDebug() << "order:";
        foreach (itemID, this->frequentItemIDsSortedByTotalSupportCount) {
            qDebug() << this->totalFrequentSupportCounts[itemID] << " times: " << Item(itemID, &(this->itemIDNameHash));
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
                transaction << Item((ItemID) this->itemNameIDHash[name], &this->itemIDNameHash);
#else
                transaction << Item((ItemID) this->itemNameIDHash[name]);
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
     * do this as fast as possible,
     * this->frequentItemIDsSortedByTotalSupportCount is reused. Because
     * this->itemIDsSortedByTotalSupportCount does not include infrequent
     * items, these are also automatically removed by this simple routine.
     *
     * @param transaction
     *   A transaction.
     * @return
     *   The optimized transaction.
     */
    Transaction FPGrowth::optimizeTransaction(const Transaction & transaction) const {
        Transaction optimizedTransaction;
        Item item;

        foreach (ItemID itemID, this->frequentItemIDsSortedByTotalSupportCount) {
            item.id = itemID;
            if (transaction.contains(item))
                optimizedTransaction.append(transaction.at(transaction.indexOf(item)));
        }

        return optimizedTransaction;
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
     * @return
     *   The entire list of frequent itemsets. The SupportCount for each Item
     *   still needs to be cleaned: it should be set to the minimum of all
     *   Items in each frequent itemset.
     */
    QList<ItemList> FPGrowth::generateFrequentItemsets(const FPTree * ctree, const ItemList & suffix) {
        QList<ItemList> frequentItemsets;
        QList<ItemList> prefixPaths;

        // First determine the suffix order for the items in this particular
        // tree, based on the list that contains *all* items in this data set,
        // sorted by support count.
        // We do this mostly for cosmetic reasons. However, it also implies
        // that we will generate frequent itemsets for which the first item is
        // the most frequent, the second item is the second most frequent, etc.
        // Note: cannot replaced with a call to FPGrowth::optimizeTransaction()
        // because that works over a QList<Item>, wheras we have to deal with
        // a QList<ItemID> here.
        ItemIDList orderedSuffixItemIDs;
        ItemIDList itemIDsInTree = ctree->getItemIDs();
        foreach (ItemID itemID, this->frequentItemIDsSortedByTotalSupportCount)
            if (itemIDsInTree.contains(itemID))
                orderedSuffixItemIDs.append(itemID);

        // We don't need the unsorted item IDs anymore.
        itemIDsInTree.clear();

        // Now iterate over each of the ordered suffix items and generate
        // frequent itemsets!
        foreach (ItemID suffixItemID, orderedSuffixItemIDs) {
            // Only if this suffix item's support meets or exceeds the minimum
            // support, it will be added as a frequent itemset (appended with
            // the received suffix of course).
            SupportCount suffixItemSupport = ctree->getItemSupport(suffixItemID);
            if (suffixItemSupport >= this->minSupportAbsolute) {
                // The current suffix item, when prepended to the received
                // suffix, is the next frequent itemset.
                // Additionally, this new frequent itemset will become the
                // next recursion's suffix.
                Item suffixItem(suffixItemID, suffixItemSupport);
#ifdef DEBUG
                suffixItem.IDNameHash = &this->itemIDNameHash;
#endif
                ItemList frequentItemset;
                frequentItemset.append(suffixItem);
                frequentItemset.append(suffix);

                // Only store the current frequent itemset if it matches the
                // constraints.
                if (this->constraints.matchItemset(frequentItemset)) {
                    frequentItemsets.append(frequentItemset);
#ifdef FPGROWTH_DEBUG
                qDebug() << "\t\t\t\t new frequent itemset:" << frequentItemset;
#endif
                }

                // Calculate the prefix paths for the current suffix item.
                prefixPaths = ctree->calculatePrefixPaths(suffixItemID);

                // Remove items from the prefix paths based that no longer
                // have sufficient support.
                prefixPaths = FPGrowth::filterPrefixPaths(prefixPaths, this->minSupportAbsolute);

                // If the conditional FP-tree won't contain the required items,
                // then just don't bother generating it.
                // This is effectively pruning the search space for frequent
                // itemsets.
                QHash<ItemID, SupportCount> prefixPathsSupportCounts = FPTree::calculateSupportCountsForPrefixPaths(prefixPaths);
                if (!this->constraints.matchSearchSpace(frequentItemset, prefixPathsSupportCounts))
                    continue;

                // If no prefix paths remain after filtering, we won't be able
                // to generate any further frequent item sets.
                if (prefixPaths.size() > 0) {
                    // Build the conditional FP-tree for these prefix paths,
                    // by creating a new FP-tree and pretending the prefix
                    // paths are transactions.
                    FPTree * cfptree = new FPTree(prefixPaths);
#ifdef FPGROWTH_DEBUG
                    qDebug() << *cfptree;
#endif

                    // Attempt to generate more frequent itemsets, with the
                    // current frequent itemset as the suffix.
                    frequentItemsets.append(this->generateFrequentItemsets(cfptree, frequentItemset));

                    delete cfptree;
                }

                // Unnecessary, but do this anyway to release memory as fast
                // as possible.
                prefixPaths.clear();
            }
        }

        return frequentItemsets;
    }
}
