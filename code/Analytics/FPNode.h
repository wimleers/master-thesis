#ifndef FPNODE_H
#define FPNODE_H

#include <QHash>
#include <QMetaType>
#include <QString>

#include "Item.h"


namespace Analytics {

    template <class T>
    class FPNode {
    public:
        FPNode(Item item = Item()) {
            this->itemID = item.id;
            this->value  = item.supportCount;
            this->parent = NULL;

#ifdef DEBUG
            this->nodeID = FPNode<T>::nextNodeID();
#endif
        }
        ~FPNode() {
            // Delete all child nodes.
            foreach (FPNode<T> * child, this->children)
                delete child;
            this->children.clear();
        }

        // Accessors.
        bool isRoot() const { return this->itemID == ROOT_ITEMID; }
        bool isLeaf() const { return this->children.size() == 0; }
        ItemID getItemID() const { return this->itemID; }
        T getValue() const { return this->value; }
        FPNode<T> * getParent() const { return this->parent; }
        FPNode<T> * getChild(ItemID itemID) const {
            if (this->children.contains(itemID))
                return this->children.value(itemID);
            else
                return NULL;
        }
        QHash<ItemID, FPNode<T> *> getChildren() const { return this->children; }
        bool hasChild(ItemID itemID) const { return this->children.contains(itemID); }
        unsigned int numChildren() const { return this->children.size(); }

        // Modifiers.
        void addChild(FPNode<T> * child) { this->children.insert(child->getItemID(), child); }
        void setParent(FPNode<T> * parent) {
            this->parent = parent;

            // Also let the parent know it has a new child, when it is a valid
            // parent.
            if (this->parent != NULL)
                this->parent->addChild(this);

        }
        void increaseValue(T count) { this->value += count; }

#ifdef DEBUG
        unsigned int getNodeID() const { return this->nodeID; }
        static void resetLastNodeID() { FPNode<T>::lastNodeID = 0; }
#endif

    protected:
        ItemID itemID;
        T value;
        FPNode<T> * parent;
        QHash<ItemID, FPNode<T> *> children;

#ifdef DEBUG
        unsigned int nodeID;
        ItemIDNameHash * itemIDNameHash;
        static unsigned int lastNodeID;
        static unsigned int nextNodeID() { return FPNode<T>::lastNodeID++; }
#endif
    };

#ifdef DEBUG
    // Initialize static members.
    template <class T>
    unsigned int FPNode<T>::lastNodeID = 0;

    template <class T>
    QDebug operator<<(QDebug dbg, const FPNode<T> & node) {
        if (node.getItemID() == ROOT_ITEMID)
            dbg.nospace() << "(NULL)";
        else {
            QString itemOutput, nodeID;

            itemOutput.clear();
            Item item(node.getItemID(), node.getValue());
            QDebug(&itemOutput) << item;

            nodeID.sprintf("0x%04d", node.getNodeID());

            dbg.nospace() << itemOutput.toStdString().c_str() << " (" << nodeID.toStdString().c_str() <<  ")";
        }

        return dbg.nospace();
    }

    template <class T>
    QDebug operator<<(QDebug dbg, const QList<FPNode<T> *> & itemPath) {
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

//template <class T>
//Q_DECLARE_METATYPE(Analytics::FPNode<T>);

#endif // FPNODE_H
