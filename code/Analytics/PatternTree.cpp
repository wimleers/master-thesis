#include "PatternTree.h"

namespace Analytics {

    //------------------------------------------------------------------------
    // Public methods.

    PatternTree::PatternTree() {
        root = new FPNode<TiltedTimeWindow>(ROOT_ITEMID);
    }

    PatternTree::~PatternTree() {
        delete root;
    }

    TiltedTimeWindow * PatternTree::getPatternSupport(const ItemIDList & pattern) const {
        return this->root->findNodeByPattern(pattern);
    }

    void PatternTree::addPattern(const FrequentItemset & pattern, uint32_t updateID) {
        // The initial current node is the root node.
        FPNode<TiltedTimeWindow> * currentNode = root;
        FPNode<TiltedTimeWindow> * nextNode;

        foreach (ItemID itemID, pattern.itemset) {
            if (currentNode->hasChild(itemID))
                nextNode = currentNode->getChild(itemID);
            else {
                // Create a new node and add it as a child of the current node.
                nextNode = new FPNode<TiltedTimeWindow>(itemID);
                nextNode->setParent(currentNode);
            }

            // We've processed this item in the transaction, time to move on
            // to the next!
            currentNode = nextNode;
            nextNode = NULL;
        }

        TiltedTimeWindow * ttw = currentNode->getPointerToValue();
        ttw->appendQuarter(pattern.support, updateID);
    }

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

            dbg.nospace() << "(" << pattern << ", " << node.getValue() << ") (" << nodeID.toStdString().c_str() <<  ")";
        }

        return dbg.nospace();
    }
#endif

}
