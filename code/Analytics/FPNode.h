#ifndef FPNODE_H
#define FPNODE_H

#include <QHash>
#include <QDebug>
#include <QMetaType>
#include <QString>

#include "typedefs.h"

namespace Analytics {
    class FPNode {
    public:
        FPNode(Item item = Item());
        ~FPNode();

        // Accessors.
        bool isRoot() const { return this->item.id == ROOT_ITEMID; }
        bool isLeaf() const { return this->children.size() == 0; }
        Item getItem() const { return this->item; }
        ItemID getItemID() const { return this->item.id; }
        SupportCount getSupportCount() const { return this->item.supportCount; }
        FPNode * getParent() const { return this->parent; }
        FPNode * getChild(ItemID itemID) const;
        ItemIDFPNodeHash getChildren() const;
        bool hasChild(ItemID itemID) const;
        unsigned int numChildren() const { return this->children.size(); }

        // Modifiers.
        void addChild(FPNode * child);
        void setItem(Item item) { this->item = item; }
        void setParent(FPNode * parent);
        void incrementSupportCount() { this->item.supportCount++; }
        void decrementSupportCount() { this->item.supportCount--; }
        void increaseSupportCount(SupportCount count) { this->item.supportCount += count; }
        void decreaseSupportCount(SupportCount count) { this->item.supportCount -= count; }

#ifdef DEBUG
        // Debug output.
        unsigned int getNodeID() const { return this->nodeID; }
        static void resetLastNodeID() { FPNode::lastNodeID = 0; }
#endif

    protected:
        FPNode * parent;
        ItemIDFPNodeHash children;
        SupportCount count;
        Item item;
#ifdef DEBUG
        unsigned int nodeID;
        ItemIDNameHash * itemIDNameHash;
        static unsigned int lastNodeID;
        static unsigned int nextNodeID() { return FPNode::lastNodeID++; }
#endif
    };

#ifdef DEBUG
    QDebug operator<<(QDebug dbg, const FPNode & node);
    QDebug operator<<(QDebug dbg, const FPNodeList & itemPath);
#endif

}

Q_DECLARE_METATYPE(Analytics::FPNode);

#endif // FPNODE_H
