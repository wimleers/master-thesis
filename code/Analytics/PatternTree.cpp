#include "PatternTree.h"

namespace Analytics {

    //------------------------------------------------------------------------
    // Public methods.

    PatternTree::PatternTree() {
        this->root = new FPNode<TiltedTimeWindow>(ROOT_ITEMID);
        this->nodeCount = 0;
        this->currentQuarter = 0;
    }

    PatternTree::~PatternTree() {
        delete root;
    }

    TiltedTimeWindow * PatternTree::getPatternSupport(const ItemIDList & pattern) const {
        return this->root->findNodeByPattern(pattern);
    }

    /**
     * Get the frequent itemsets that match given constraints for a range of
     * buckets in the TiltedTimeWindows in this PatternTree.
     *
     * @param minSupport
     *   The minimum support that the itemset must have over the given range
     *   to qualify as "frequent".
     * @param frequentItemsetConstraints
     *   The constraints that frequent itemsets must match.
     * @param from
     *   The range starts at this bucket.
     * @param to
     *   The range starts at this bucket.
     * @param prefix
     *   Internal parameter (for recursive calls).
     * @param node
     *   Internal parameter (for recursive calls).
     * @return
     *   The frequent itemsets over the given range that match the given
     *   constraints.
     */
    QList<FrequentItemset> PatternTree::getFrequentItemsetsForRange(SupportCount minSupport, const Constraints & frequentItemsetConstraints, uint from, uint to, const ItemIDList & prefix, FPNode<TiltedTimeWindow> * node) const {
        QList<FrequentItemset> frequentItemsets;
        FrequentItemset frequentItemset;

        // Start at the root.
        if (node == NULL)
            node = this->root;
        // If it's not the root node, set the current frequent itemset.
        else {
            frequentItemset.itemset = prefix;
            frequentItemset.itemset.append(node->getItemID());
            frequentItemset.support = node->getValue().getSupportForRange(from, to);
#ifdef DEBUG
            frequentItemset.IDNameHash = node->itemIDNameHash;
#endif
        }

        // Add this frequent itemset to the list of frequent itemsets if
        // it qualifies through its support and if it matches the
        // constraints.
        if (frequentItemset.support > minSupport && frequentItemsetConstraints.matchItemset(frequentItemset.itemset))
            frequentItemsets.append(frequentItemset);

        // Recursive call for each child node of the current node.
        foreach (FPNode<TiltedTimeWindow> * child, node->getChildren()) {
            frequentItemsets.append(this->getFrequentItemsetsForRange(
                    minSupport,
                    frequentItemsetConstraints,
                    from,
                    to,
                    frequentItemset.itemset,
                    child
            ));
        }

        return frequentItemsets;
    }

    void PatternTree::addPattern(const FrequentItemset & pattern, quint32 updateID) {
        // The initial current node is the root node.
        FPNode<TiltedTimeWindow> * currentNode = root;
        FPNode<TiltedTimeWindow> * nextNode;

        foreach (ItemID itemID, pattern.itemset) {
            if (currentNode->hasChild(itemID))
                nextNode = currentNode->getChild(itemID);
            else {
                // Create a new node and add it as a child of the current node.
                nextNode = new FPNode<TiltedTimeWindow>(itemID);
                this->nodeCount++;
                nextNode->setParent(currentNode);
#ifdef DEBUG
        nextNode->itemIDNameHash = pattern.IDNameHash;
#endif
            }

            // We've processed this item in the transaction, time to move on
            // to the next!
            currentNode = nextNode;
            nextNode = NULL;
        }

        TiltedTimeWindow * ttw = currentNode->getPointerToValue();

        // Make sure the quarters are in sync.
        for (uint i = ttw->getCapacityUsed(GRANULARITY_QUARTER); i < this->currentQuarter; i++)
            ttw->appendQuarter(0, updateID);

        // Now that the quarters are in sync, finally append the quarter.
        ttw->appendQuarter(pattern.support, updateID);
    }

    void PatternTree::removePattern(FPNode<TiltedTimeWindow> * const node) {
        this->nodeCount -= (1 + node->getNumDescendants());
        delete node;
    }


    //------------------------------------------------------------------------
    // Static public methods.

    ItemIDList PatternTree::getPatternForNode(FPNode<TiltedTimeWindow> const * const node) {
        ItemIDList pattern;
        FPNode<TiltedTimeWindow> const * nextNode;

        nextNode = node;
        while (nextNode->getItemID() != ROOT_ITEMID) {
            pattern.prepend(nextNode->getItemID());
            nextNode = nextNode->getParent();
        }

        return pattern;
    }


    //------------------------------------------------------------------------
    // Other.

#ifdef DEBUG
    QDebug operator<<(QDebug dbg, const PatternTree & tree) {
        dbg.nospace() << dumpHelper(*(tree.getRoot())).toStdString().c_str();

        return dbg.nospace();
    }

    QString dumpHelper(const FPNode<TiltedTimeWindow> & node, QString prefix) {
        static QString suffix = "\t";
        QString s;
        bool firstChild = true;

        // Print current node.
        QDebug(&s) << node << "\n";

        // Print all child nodes.
        if (node.numChildren() > 0) {
            foreach (FPNode<TiltedTimeWindow> * child, node.getChildren()) {
                if (firstChild)
                    s += prefix;
                else
                    firstChild = false;
                s += "-> " + dumpHelper(*child, prefix + suffix);
            }
        }

        return s;
    }

    QDebug operator<<(QDebug dbg, const FPNode<TiltedTimeWindow> & node) {
        if (node.getItemID() == ROOT_ITEMID)
            dbg.nospace() << "(NULL)";
        else {
            QString nodeID;

            ItemIDList pattern = PatternTree::getPatternForNode(&node);
            nodeID.sprintf("0x%04d", node.getNodeID());

            dbg.nospace() << "({";
            itemIDHelper(dbg, pattern, node.itemIDNameHash);
            dbg.nospace() << "}, " << node.getValue() << ") (" << nodeID.toStdString().c_str() <<  ")";
        }

        return dbg.nospace();
    }
#endif

}
