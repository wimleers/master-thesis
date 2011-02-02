#include "TestFPTree.h"


void TestFPTree::basic() {
    FPTree * tree = new FPTree();

    // Build ItemIDNameHash;
    ItemIDNameHash itemIDNameHash;
    ItemIDNameHash * hash = &itemIDNameHash;
    itemIDNameHash.insert(1, "A");
    itemIDNameHash.insert(2, "B");
    itemIDNameHash.insert(3, "C");
    itemIDNameHash.insert(4, "D");

    // Create a few transactions.
    Transaction t1, t2, t3, t4;

    t1 << Item(1, hash) << Item(2, hash);
    t2 << Item(2, hash) << Item(3, hash);
    t3 << Item(1, hash) << Item(2, hash) << Item(3, hash);
    t4 << Item(1, hash) << Item(4, hash);

    tree->addTransaction(t1);
    tree->addTransaction(t2);
    tree->addTransaction(t3);
    tree->addTransaction(t4);

    // Helpful for debugging/expanding this test.
    //qDebug() << *tree;

    // Verify the available item paths.
    QCOMPARE(tree->getItemIDs(), QList<ItemID>() << 1 << 2 << 3 << 4);
    FPNodeList itemPath;
    // Item path for A(1): A(1)=3 (0x0001)
    itemPath = tree->getItemPath(1);
    QCOMPARE(itemPath.size(), 1);
    QCOMPARE(itemPath[0]->getNodeID(), (unsigned int) 1);
    QCOMPARE(itemPath[0]->getSupportCount(), (SupportCount) 3);
    // Item path for B(2): B(2)=2 (0x0002) -> B(2)=1 (0x0003)
    itemPath = tree->getItemPath(2);
    QCOMPARE(itemPath.size(), 2);
    QCOMPARE(itemPath[0]->getNodeID(), (unsigned int) 2);
    QCOMPARE(itemPath[0]->getSupportCount(), (SupportCount) 2);
    QCOMPARE(itemPath[1]->getNodeID(), (unsigned int) 3);
    QCOMPARE(itemPath[1]->getSupportCount(), (SupportCount) 1);
    // Item path for C(3): C(3)=1 (0x0004) -> C(3)=1 (0x0005)
    itemPath = tree->getItemPath(3);
    QCOMPARE(itemPath.size(), 2);
    QCOMPARE(itemPath[0]->getNodeID(), (unsigned int) 4);
    QCOMPARE(itemPath[0]->getSupportCount(), (SupportCount) 1);
    QCOMPARE(itemPath[1]->getNodeID(), (unsigned int) 5);
    QCOMPARE(itemPath[1]->getSupportCount(), (SupportCount) 1);
    // Item path for D(4): D(4)=1 (0x0006)
    itemPath = tree->getItemPath(4);
    QCOMPARE(itemPath.size(), 1);
    QCOMPARE(itemPath[0]->getNodeID(), (unsigned int) 6);
    QCOMPARE(itemPath[0]->getSupportCount(), (SupportCount) 1);


    // Verify the total item support counts.
    QCOMPARE(tree->getItemSupport(1), (SupportCount) 3);
    QCOMPARE(tree->getItemSupport(2), (SupportCount) 3);
    QCOMPARE(tree->getItemSupport(3), (SupportCount) 2);
    QCOMPARE(tree->getItemSupport(4), (SupportCount) 1);


    // Verify the tree shape.
    FPNode * node;
    FPNode * root = tree->getRoot();
    QCOMPARE(root->getNodeID(), (unsigned int) 0);
    QCOMPARE(root->getItemID(), (ItemID) ROOT_ITEMID);

    // First branch.
    // root -> A(1)=3 (0x0001)
    node = root->getChild(1);
    QVERIFY(node != NULL);
    QCOMPARE(node->getSupportCount(), (SupportCount) 3);
    QCOMPARE(node->getNodeID(), (unsigned int) 1);
    FPNode * firstBranch = node;
    // root -> A(1)=3 (0x0001) -> B(2)=2 (0x0002)
    node = firstBranch->getChild(2);
    QVERIFY(node != NULL);
    QCOMPARE(node->getSupportCount(), (SupportCount) 2);
    QCOMPARE(node->getNodeID(), (unsigned int) 2);
    // root -> A(1)=3 (0x0001) -> C(3)=1 (0x0005)
    node = node->getChild(3);
    QVERIFY(node != NULL);
    QCOMPARE(node->getSupportCount(), (SupportCount) 1);
    QCOMPARE(node->getNodeID(), (unsigned int) 5);
    // root -> A(1)=3 (0x0001) -> D(4)=1 (0x0006)
    node = firstBranch->getChild(4);
    QVERIFY(node != NULL);
    QCOMPARE(node->getSupportCount(), (SupportCount) 1);
    QCOMPARE(node->getNodeID(), (unsigned int) 6);

    // Second branch.
    // root -> B(2)=1 (0x0003)
    node = root->getChild(2);
    QVERIFY(node != NULL);
    QCOMPARE(node->getSupportCount(), (SupportCount) 1);
    QCOMPARE(node->getNodeID(), (unsigned int) 3);
    // root -> B(2)=1 (0x0003) -> C(3)=1 (0x0004)
    node = node->getChild(3);
    QVERIFY(node != NULL);
    QCOMPARE(node->getSupportCount(), (SupportCount) 1);
    QCOMPARE(node->getNodeID(), (unsigned int) 4);

    delete tree;
}
