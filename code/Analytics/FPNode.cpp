#include "FPNode.h"

namespace Analytics {

#ifdef DEBUG
    // Initialize static members.
    unsigned int FPNode::lastNodeID = 0;
#endif

    FPNode::FPNode(Item item) {
        this->item = item;
        this->parent = NULL;

#ifdef DEBUG
        this->nodeID = FPNode::nextNodeID();
#endif
    }

    FPNode::~FPNode() {
        // Delete all child nodes.
        foreach (FPNode * child, this->children)
            delete child;
        this->children.clear();
    }

    bool FPNode::hasChild(ItemID itemID) const {
        return this->children.contains(itemID);
    }

    FPNode * FPNode::getChild(ItemID itemID) const {
        if (this->children.contains(itemID))
            return this->children.value(itemID);
        else
            return NULL;
    }

    ItemIDFPNodeHash FPNode::getChildren() const {
        return this->children;
    }

    void FPNode::addChild(FPNode * child) {
        this->children.insert(child->getItemID(), child);
    }

    void FPNode::setParent(FPNode * parent) {
        this->parent = parent;

        // Also let the parent know it has a new child, when it is a valid
        // parent.
        if (this->parent != NULL)
            this->parent->addChild(this);
    }


#ifdef DEBUG
    QDebug operator<<(QDebug dbg, const FPNode &node) {
        if (node.getItemID() == ROOT_ITEMID)
            dbg.nospace() << "(NULL)";
        else {
            QString itemOutput, nodeID;

            itemOutput.clear();
            QDebug(&itemOutput) << node.getItem();

            nodeID.sprintf("0x%04d", node.getNodeID());

            dbg.nospace() << itemOutput.toStdString().c_str() << " (" << nodeID.toStdString().c_str() <<  ")";
        }

        return dbg.nospace();
    }

    QDebug operator<<(QDebug dbg, const FPNodeList &itemPath) {
        dbg.nospace() << "[size=" << itemPath.size() << "] ";

        for (int i = 0; i < itemPath.size(); i++) {
            if (i > 0)
                dbg.nospace() << " -> ";
            dbg.nospace() << *(itemPath[i]);
        }

        return dbg.nospace();
    }
#endif

}
