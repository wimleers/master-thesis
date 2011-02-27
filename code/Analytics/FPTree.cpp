#include "FPTree.h"

namespace Analytics {

    //------------------------------------------------------------------------
    // Public methods.

    FPTree::FPTree() {
        this->init();
    }

    // Alternative constructor
    FPTree::FPTree(const QList<ItemList> & prefixPaths) {
        this->init();

        // Now insert all the prefix paths as transactions.
        foreach (ItemList prefixPath, prefixPaths)
            this->addTransaction(prefixPath);
    }

    FPTree::~FPTree() {
        delete root;
    }

    bool FPTree::hasItemPath(ItemID itemID) const {
        return this->itemPaths.contains(itemID);
    }

    FPNodeList FPTree::getItemPath(ItemID itemID) const {
        if (this->itemPaths.contains(itemID))
            return this->itemPaths[itemID];
        else {
            FPNodeList empty;
            return empty;
        }
    }

    bool FPTree::itemPathContains(ItemID itemID, FPNode * node) const {
        return (this->itemPaths.contains(itemID) && this->itemPaths[itemID].contains(node));
    }

    SupportCount FPTree::getItemSupport(ItemID itemID) const {
        SupportCount supportCount = 0;
        foreach (FPNode * node, this->itemPaths[itemID])
            supportCount += node->getSupportCount();
        return supportCount;
    }

    /**
     * Calculate prefix paths that end with a node that has the given ItemID.
     * These nodes can be retrieved very quickly using the FPTree's itemPaths.
     * A prefix path is a list of Items that reflects a path from the bottom
     * of the tree to the root (but excluding the root), following along the
     * path of an FPNode that has the ItemID itemID. Because it is a list of
     * Items, it also includes both the ItemID and the SupportCount. The
     * original SupportCount of the Item encapsulated by the FPNode is erased
     * and replaced by the SupportCount of the FPNode we started from, i.e. a
     * node that has the ItemID itemID, because we're looking at only the
     * paths that include this node.
     * Exclude the leaf node itself, as it will no longer be needed.
     */
    QList<ItemList> FPTree::calculatePrefixPaths(ItemID itemID) const {
        QList<ItemList> prefixPaths;
        ItemList prefixPath;
        FPNode * node;
        SupportCount supportCount;
        Item item;

        FPNodeList leafNodes = this->getItemPath(itemID);
        foreach (FPNode * leafNode, leafNodes) {
            // Build the prefix path starting from the given leaf node, by
            // traversing up the tree (but do not include the leaf node's item
            // in the prefix path).
            // Don't copy the item's original count, but the count of the leaf
            // node instead, because we're looking at only the paths that
            // include this leaf node.
            node = leafNode;
            supportCount = leafNode->getSupportCount();
            while ((node = node->getParent()) != NULL && node->getItemID() != ROOT_ITEMID) {
                item = node->getItem();
                item.supportCount = supportCount;
                prefixPath.prepend(item);
            }

            // Store the built prefix path & clear it, so we can calculate the
            // next. Of course only if there *is* a prefix path, which is not
            // the case if the given itemID is at the root level.
            if (prefixPath.size() > 0) {
                prefixPaths.append(prefixPath);
                prefixPath.clear();
            }
        }

        return prefixPaths;
    }

    QHash<ItemID, SupportCount> FPTree::calculateSupportCountsForPrefixPaths(QList<ItemList> prefixPaths) {
        QHash<ItemID, SupportCount> supportCounts;

        foreach (ItemList prefixPath, prefixPaths)
            foreach (Item item, prefixPath)
                supportCounts[item.id] = (supportCounts.contains(item.id)) ? supportCounts[item.id] + item.supportCount : item.supportCount;

        return supportCounts;
    }

    void FPTree::addTransaction(Transaction transaction) {
        // The initial current node is the root node.
        FPNode * currentNode = root;

        FPNode * nextNode;

        foreach (Item item, transaction) {
            if (currentNode->hasChild(item.id)) {
                // There is already a node in the tree for the current
                // transaction item, so reuse it: increase its support count.
                nextNode = currentNode->getChild(item.id);
                nextNode->increaseSupportCount(item.supportCount);
            }
            else {
                // Create a new node and add it as a child of the current node.
                nextNode = new FPNode(item);
                nextNode->setParent(currentNode);

                // Update the item path to include the new node.
                this->addNodeToItemPath(nextNode);
            }

            // We've processed this item in the transaction, time to move on
            // to the next!
            currentNode = nextNode;
            nextNode = NULL;
        }
    }


    //------------------------------------------------------------------------
    // Protected methods.

    void FPTree::init() {
        Item rootItem;
        rootItem.id = ROOT_ITEMID;
        rootItem.supportCount = 0;
        root = new FPNode(rootItem);
    }

    void FPTree::addNodeToItemPath(FPNode * node) {
        FPNodeList itemPath;

        ItemID itemID = node->getItemID();

        // If there already is an item path for this item, load it so it can
        // be updated.
        if (this->itemPaths.contains(itemID))
            itemPath = this->itemPaths[itemID];

        itemPath.append(node);
        this->itemPaths.insert(itemID, itemPath);
    }


    //------------------------------------------------------------------------
    // Other.

#ifdef DEBUG
    QDebug operator<<(QDebug dbg, const FPTree &tree) {
        // Tree.
        dbg.nospace() << "TREE" << endl;
        dbg.nospace() << dumpHelper(*(tree.getRoot())).toStdString().c_str();
        dbg.nospace() << endl;

        // Item paths.
        dbg.nospace() << "ITEM PATHS (size indicates the number of branches this item occurs in)" << endl;
        FPNodeList itemPath;
        Item item;
        foreach (ItemID itemID, tree.getItemIDs()) {
            itemPath = tree.getItemPath(itemID);
            item = itemPath[0]->getItem();
            dbg.nospace() << " - item path for "
                    << item.IDNameHash->value(itemPath[0]->getItemID()).toStdString().c_str()
                    << ": " << itemPath << endl;
        }

        return dbg.nospace();
    }

    QString dumpHelper(const FPNode &node, QString prefix) {
        static QString suffix = "\t";
        QString s;
        bool firstChild = true;

        // Print current node.
        QDebug(&s) << node << "\n";

        // Print all child nodes.
        if (node.numChildren() > 0) {
            foreach (FPNode * child, node.getChildren()) {
                if (firstChild)
                    s += prefix;
                else
                    firstChild = false;
                s += "-> " + dumpHelper(*child, prefix + suffix);
            }
        }

        return s;
    }
#endif

}
