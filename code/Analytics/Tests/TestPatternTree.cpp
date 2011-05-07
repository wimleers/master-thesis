#include "TestPatternTree.h"

void TestPatternTree::basic() {
    PatternTree * patternTree = new PatternTree();

    // Pattern 1: {1, 2, 3}, support: 1.
    ItemIDList p1;
    p1 << 1 << 2 << 3;
    SupportCount s1 = 1;
    patternTree->addPattern(FrequentItemset(p1, s1));

    // Pattern 2: {1, 2}, support: 2, add this twice..
    ItemIDList p2;
    p2 << 1 << 2;
    SupportCount s2 = 2;
    patternTree->addPattern(FrequentItemset(p2, s2));
    patternTree->addPattern(FrequentItemset(p2, s2));

    // Pattern 3: {1, 4}, support: 5.
    ItemIDList p3;
    p3 << 1 << 4;
    SupportCount s3 = 5;
    patternTree->addPattern(FrequentItemset(p3, s3));

    // Helpful for debugging/expanding this test.
    // Currently, this should match:
    // (NULL)
    // -> ({1}, {}) (0x0001)
    //         -> ({1, 2}, {Q={2, 2}}) (0x0002)
    //                 -> ({1, 2, 3}, {Q={1}}) (0x0003)
    //         -> ({1, 4}, {Q={5}}) (0x0004)
    //qDebug() << *patternTree;

    // Verify the tree shape.
    FPNode<TiltedTimeWindow> * node;
    FPNode<TiltedTimeWindow> * root = patternTree->getRoot();
    QCOMPARE(root->getNodeID(), (unsigned int) 0);
    QCOMPARE(root->getItemID(), (ItemID) ROOT_ITEMID);

    // First branch.
    // root -> ({1}, {}) (0x0001)
    node = root->getChild(1);
    ItemIDList referencePattern = ItemIDList() << 1;
    QVector<SupportCount> referenceBuckets = QVector<SupportCount>();
    QVERIFY(node != NULL);
    QCOMPARE(node->getItemID(), (ItemID) 1);
    QCOMPARE(node->getValue().getBuckets(0), referenceBuckets);
    QCOMPARE(node->getNodeID(), (unsigned int) 1);
    QCOMPARE(PatternTree::getPatternForNode(node), referencePattern);
    QCOMPARE(patternTree->getPatternSupport(referencePattern)->getBuckets(0), referenceBuckets);
    FPNode<TiltedTimeWindow> * splitNode = node;
    // root -> ({1}, {}) (0x0001) -> ({1, 2}, {Q={2, 2}}) (0x0002)
    node = node->getChild(2);
    referencePattern = ItemIDList() << 1 << 2;
    referenceBuckets = QVector<SupportCount>() << 2 << 2;
    QVERIFY(node != NULL);
    QCOMPARE(node->getItemID(), (ItemID) 2);
    QCOMPARE(node->getValue().getBuckets(2), referenceBuckets);
    QCOMPARE(node->getNodeID(), (unsigned int) 2);
    QCOMPARE(PatternTree::getPatternForNode(node), referencePattern);
    QCOMPARE(patternTree->getPatternSupport(referencePattern)->getBuckets(2), referenceBuckets);
    // root -> ({1}, {}) (0x0001) -> ({1, 2}, {Q={2, 2}}) (0x0002) -> ({1, 2, 3}, {Q={1}}) (0x0003)
    node = node->getChild(3);
    referencePattern = ItemIDList() << 1 << 2 << 3;
    referenceBuckets = QVector<SupportCount>() << 1;
    QVERIFY(node != NULL);
    QCOMPARE(node->getItemID(), (ItemID) 3);
    QCOMPARE(node->getValue().getBuckets(1), referenceBuckets);
    QCOMPARE(node->getNodeID(), (unsigned int) 3);
    QCOMPARE(PatternTree::getPatternForNode(node), referencePattern);
    QCOMPARE(patternTree->getPatternSupport(referencePattern)->getBuckets(1), referenceBuckets);

    // Second branch.
    // root -> ({1}, {}) (0x0001) -> ({1, 4}, {Q={5}}) (0x0004)
    node = splitNode->getChild(4);
    referencePattern = ItemIDList() << 1 << 4;
    referenceBuckets = QVector<SupportCount>() << 5;
    QVERIFY(node != NULL);
    QCOMPARE(node->getItemID(), (ItemID) 4);
    QCOMPARE(node->getValue().getBuckets(1), referenceBuckets);
    QCOMPARE(node->getNodeID(), (unsigned int) 4);
    QCOMPARE(PatternTree::getPatternForNode(node), referencePattern);
    QCOMPARE(patternTree->getPatternSupport(referencePattern)->getBuckets(1), referenceBuckets);

    delete patternTree;
}
