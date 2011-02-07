#include "FPGrowth.h"

namespace Analytics {

    FPGrowth::FPGrowth(const QList<QStringList> & transactions, float minSupportRelative) {
        this->transactions = transactions;

        this->minSupportRelative = minSupportRelative;
        this->minSupportAbsolute = ceil(this->minSupportRelative * this->transactions.size());

        this->tree = new FPTree();
    }

    FPGrowth::~FPGrowth() {
        delete this->tree;
    }

    /**
     * Set the requirements for transactions: each transaction needs to have
     * all of the required items. Wildcards are allowed, e.g. "episode:*" will
     * match "episode:foo", "episode:bar", etc.
     *
     * @param requiredItems
     *   A list of required transaction items.
     */
    void FPGrowth::setTransactionRequirements(QList<ItemName> items) {
        this->transactionRequirements = items;
    }

    /**
     * Mine frequent itemsets. (First scan the transactions, then build the
     * FP-tree, then generate the frequent itemsets from there.)
     */
    QList<ItemList> FPGrowth::mineFrequentItemsets() {
        this->scanTransactions();
        this->buildFPTree();
        return this->generateFrequentItemsets(this->tree);
    }


    //------------------------------------------------------------------------------
    // Protected slots.

    /**
     * Slot that receives a Transaction, optimizes it and adds it to the tree.
     */
    void FPGrowth::processTransaction(Transaction transaction) {
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
     * Given an ItemID -> SupportCount dictionary, find sort ItemIDs by
     * decreasing support count.
     *
     * @param itemSupportCounts
     *   A QHash of ItemID -> SupportCount pairs.
     * @return
     *   A QList of ItemIDs, sorted by decreasing support count.
     */
    QList<ItemID> FPGrowth::sortItemIDsByDecreasingSupportCount(QHash<ItemID, SupportCount> itemSupportCounts){
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
                }
                else
                    itemID = this->itemNameIDHash[itemName];

                this->totalFrequentSupportCounts[itemID]++;
            }
        }

        // Discard infrequent items' SupportCount.
        foreach (ItemID id, this->totalFrequentSupportCounts.keys())
            if (this->totalFrequentSupportCounts[id] < this->minSupportAbsolute)
                this->totalFrequentSupportCounts.remove(id);

        // Sort the frequent items' ItemIDs by decreasing SupportCount.
        this->frequentItemIDsSortedByTotalSupportCount = FPGrowth::sortItemIDsByDecreasingSupportCount(this->totalFrequentSupportCounts);
    }

    /**
     * Build the FP-tree, by using the results from scanTransactions() and
     * taking into account the transactions requirements set through
     * setTransactionRequirements().
     */
    void FPGrowth::buildFPTree() {
        Transaction transaction;
        bool matchesFilter;

        // When filter items have been set, require all transactions to match
        // either of these filter items.
        bool requiresFilterMatch = (this->transactionRequirements.size() > 0);

        foreach (QStringList transactionAsStrings, this->transactions) {
            matchesFilter = false;
            transaction.clear();
            foreach (ItemName name, transactionAsStrings) {
                // Filter transactions when appropriate.
                // TODO: wildcard matches (e.g. "episode:*")
                if (requiresFilterMatch && !matchesFilter) {
                    foreach (ItemName filter, this->transactionRequirements)
                        if (filter.compare(name) == 0)
                            matchesFilter = true;
                }
#ifdef DEBUG
                transaction << Item((ItemID) this->itemNameIDHash[name], &this->itemIDNameHash);
#else
                transaction << Item((ItemID) this->itemNameIDHash[name]);
#endif
            }

            if (!requiresFilterMatch || matchesFilter)
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
    Transaction FPGrowth::optimizeTransaction(Transaction transaction) const {
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
    QList<ItemList> FPGrowth::generateFrequentItemsets(FPTree * ctree, ItemList suffix) {
#ifdef FPGROWTH_DEBUG
        qDebug() << "---------------------------------generateFrequentItemsets()" << suffix;
#endif

        QList<ItemList> frequentItemsets;
        QList<ItemList> prefixPaths;

        // First determine the suffix order for the items in this particular
        // tree, based on the list that contains *all* items in this data set,
        // sorted by support count.
        // TODO: Why does the order of the suffixes mather at all? We have to
        // generate all of them anyway. Least or most support count first, it
        // does not matter. Is this merely cosmetic, i.e. to always generate
        // suffixes in analogous orders for different branches?
        ItemIDList orderedSuffixItemIDs;
        ItemIDList itemIDsInTree = ctree->getItemIDs();
        foreach (ItemID itemID, this->frequentItemIDsSortedByTotalSupportCount)
            if (itemIDsInTree.contains(itemID))
                orderedSuffixItemIDs.append(itemID);

        // Now iterate over each of the ordered suffix items and generate
        // frequent itemsets!
        foreach (ItemID suffixItemID, orderedSuffixItemIDs) {
#ifdef FPGROWTH_DEBUG
            if (suffix.size() == 0)
                qDebug() << "==========ROOT==========";
            Item suffixItem(suffixItemID, ctree->getItemSupport(suffixItemID), &this->itemIDNameHash);
            qDebug() << "suffix item" << suffixItem << ", meets min sup? " << (ctree->getItemSupport(suffixItemID) >= this->minimumSupport);

#endif

            // Only if this suffix item's support meets or exceeds the minim
            // support, it will be added as a frequent itemset (appended with
            // the received suffix of course).
            SupportCount suffixItemSupport = ctree->getItemSupport(suffixItemID);
            if (suffixItemSupport >= this->minSupportAbsolute) {
                // The current suffix item, when prepended to the received
                // suffix, is the next frequent itemset. Additionally, it will
                // serve as the next suffix.
                Item suffixItem(suffixItemID, suffixItemSupport);
#ifdef DEBUG
                suffixItem.IDNameHash = &this->itemIDNameHash;
#endif
                ItemList frequentItemset;
                frequentItemset.append(suffixItem);
                frequentItemset.append(suffix);

#ifdef FPGROWTH_DEBUG
                qDebug() << "\t\t\t\t new frequent itemset:" << frequentItemset;
#endif

                // Add the new frequent itemset to the list of frequent
                // itemsets.
                frequentItemsets.append(frequentItemset);

                // Calculate the prefix paths for the current suffix item.
                prefixPaths = ctree->calculatePrefixPaths(suffixItemID);

#ifdef FPGROWTH_DEBUG
                qDebug() << "prefix paths:";
                qDebug() << prefixPaths;
#endif

                // Calculate the support counts for the prefix paths.
                QHash<ItemID, SupportCount> prefixPathsSupportCounts = FPTree::calculateSupportCountsForPrefixPaths(prefixPaths);

                // Remove items from the prefix paths based on the support
                // counts of the items *within* the prefix paths.
                QList<ItemList> filteredPrefixPaths;
                ItemList filteredPrefixPath;
                foreach (ItemList prefixPath, prefixPaths) {
                    foreach (Item item, prefixPath) {
                        if (prefixPathsSupportCounts[item.id] >= this->minSupportAbsolute)
                            filteredPrefixPath.append(item);
                    }
                    if (filteredPrefixPath.size() > 0)
                        filteredPrefixPaths.append(filteredPrefixPath);
                    filteredPrefixPath.clear();
                }

#ifdef FPGROWTH_DEBUG
                qDebug() << "filtered prefix paths: ";
                qDebug() << filteredPrefixPaths;
#endif

                // If no prefix paths remain after filtering, we won't be able
                // to generate any further frequent item sets.
                if (filteredPrefixPaths.size() > 0) {
                    // Build the conditional FP-tree for these prefix paths,
                    // by creating a new FP-tree and pretending the prefix
                    // paths are transactions.
                    FPTree * cfptree = new FPTree();
                    foreach (ItemList prefixPath, filteredPrefixPaths)
                        cfptree->addTransaction(prefixPath);
#ifdef FPGROWTH_DEBUG
                    qDebug() << *cfptree;
#endif

                    // Attempt to generate more frequent itemsets, with the
                    // current frequent itemset as the suffix.
                    frequentItemsets.append(this->generateFrequentItemsets(cfptree, frequentItemset));

                    delete cfptree;
                }
            }
#ifdef FPGROWTH_DEBUG
            else
                qDebug() << "support count of" << suffixItem << "in the initial tree is less than minimum support";
#endif
        }
#ifdef FPGROWTH_DEBUG
        qDebug() << "------END------------------------generateFrequentItemsets()" << suffix;
#endif

        return frequentItemsets;
    }
}
