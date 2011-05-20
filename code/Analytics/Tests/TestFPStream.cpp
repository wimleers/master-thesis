#include "TestFPStream.h"

void TestFPStream::calculateDroppableTail() {
    TiltedTimeWindow ttw;
    ttw.appendQuarter(1, 0);
    ttw.appendQuarter(0, 1);
    ttw.appendQuarter(0, 2);
    ttw.appendQuarter(0, 3);
    ttw.appendQuarter(0, 4);

    TiltedTimeWindow batchSizes;
    for (int i = 0; i < 5; i++)
        batchSizes.appendQuarter(TTW_BUCKET_UNUSED, i);

    // Helpful for debugging/expanding this test.
    // Currently, this should match:
    // {Q={0}, H={1}} (lastUpdate=4), buckets: QVector(0, 4294967295, 4294967295, 4294967295, 1)
    //qDebug() << ttw << ", buckets: " << ttw.getBuckets(5);

    // Not lower than minimum support.
    // - min sup: 1 < ceil(0.4 * 2)  <=>  1 < 1  <=>  false
    batchSizes.buckets[4] = 2;
    QCOMPARE(FPStream::calculateDroppableTail(ttw, 0.4, 0.05, batchSizes), -1);

    // Lower than minimum support, but not lower than cumulative maximum
    // support error.
    // - min sup: 1 < ceil(0.4 * 3)  <=>  1 < 2  <=>  true
    // - max err: 1 < ceil(0.05* 3)  <=>  1 < 1  <=>  false
    batchSizes.buckets[4] = 3;
    QCOMPARE(FPStream::calculateDroppableTail(ttw, 0.4, 0.05, batchSizes), -1);

    // Lower than minimum support, but not lower than cumulative maximum
    // support error (although it is equal and thus barely not lower!).
    // - min sup: 1 < ceil(0.4 * 20)  <=>  1 < 8  <=>  true
    // - max err: 1 < ceil(0.05* 20)  <=>  1 < 1  <=>  false
    batchSizes.buckets[4] = 20;
    QCOMPARE(FPStream::calculateDroppableTail(ttw, 0.4, 0.05, batchSizes), -1);

    // Lower than minimum support, and also lower than cumulative maximum
    // support error.
    // - min sup: 1 < ceil(0.4 * 21)  <=>  1 < 9  <=>  true
    // - max err: 1 < ceil(0.05* 21)  <=>  1 < 2  <=>  true
    batchSizes.buckets[4] = 21;
    QCOMPARE(FPStream::calculateDroppableTail(ttw, 0.4, 0.05, batchSizes), 4);
}

void TestFPStream::basic() {
    ItemIDNameHash itemIDNameHash;
    ItemNameIDHash itemNameIDHash;
    ItemIDList sortedFrequentItemIDs;
    FPNode<TiltedTimeWindow>::resetLastNodeID();
    FPStream * fpstream = new FPStream(0.4, 0.05, &itemIDNameHash, &itemNameIDHash, &sortedFrequentItemIDs);

    // First batch of transactions.
    QList<QStringList> transactions;
    transactions.append(QStringList() << "A" << "B" << "C" << "D");
    transactions.append(QStringList() << "A" << "B");
    transactions.append(QStringList() << "A" << "C");
    transactions.append(QStringList() << "A" << "B" << "C");
    transactions.append(QStringList() << "A" << "D");
    transactions.append(QStringList() << "A" << "C" << "D");
    transactions.append(QStringList() << "C" << "B");
    transactions.append(QStringList() << "B" << "C");
    transactions.append(QStringList() << "C" << "D");
    transactions.append(QStringList() << "C" << "E");

    fpstream->processBatchTransactions(transactions);

    // Helpful for debugging/expanding this test.
    // Currently, this should match:
    // (NULL)
    // -> ({A(0)}, {Q={6}}) (0x0007)
    //     -> ({A(0), B(1)}, {Q={3}}) (0x0011)
    //         -> ({A(0), B(1), D(3)}, {Q={1}}) (0x0019)
    //     -> ({A(0), D(3)}, {Q={3}}) (0x0015)
    // -> ({B(1)}, {Q={5}}) (0x0009)
    //     -> ({B(1), D(3)}, {Q={1}}) (0x0017)
    // -> ({C(2)}, {Q={8}}) (0x0006)
    //     -> ({C(2), A(0)}, {Q={4}}) (0x0008)
    //         -> ({C(2), A(0), B(1)}, {Q={2}}) (0x0012)
    //             -> ({C(2), A(0), B(1), D(3)}, {Q={1}}) (0x0020)
    //         -> ({C(2), A(0), D(3)}, {Q={2}}) (0x0016)
    //     -> ({C(2), B(1)}, {Q={4}}) (0x0010)
    //         -> ({C(2), B(1), D(3)}, {Q={1}}) (0x0018)
    //     -> ({C(2), D(3)}, {Q={3}}) (0x0014)
    //     -> ({C(2), E(4)}, {Q={1}}) (0x0022)
    // -> ({D(3)}, {Q={4}}) (0x0013)
    // -> ({E(4)}, {Q={1}}) (0x0021)
    const PatternTree & patternTree = fpstream->getPatternTree();
    QCOMPARE(patternTree.getNodeCount(), (unsigned int) 17);
    //qDebug() << fpstream->getPatternTree();

    // Verify the tree shape.
    FPNode<TiltedTimeWindow> * node;
    FPNode<TiltedTimeWindow> * root = patternTree.getRoot();
    QCOMPARE(root->getNodeID(), (unsigned int) 0);
    QCOMPARE(root->getItemID(), (ItemID) ROOT_ITEMID);

    // First branch.
    // root -> ({A(0)}, {Q={6}}) (0x0002)
    node = root->getChild(0);
    this->verifyNode(patternTree, node, 0, 2, ItemIDList() << 0, QVector<SupportCount>() << 6);
    // root -> ({A(0)}, {Q={6}}) (0x0002) -> ({A(0), B(1)}, {Q={3}}) (0x0006)
    node = node->getChild(1);
    this->verifyNode(patternTree, node, 1, 6, ItemIDList() << 0 << 1, QVector<SupportCount>() << 3);
    // root -> ({A(0)}, {Q={6}}) (0x0002) -> ({A(0), B(1)}, {Q={3}}) (0x0006) -> ({A(0), B(1), D(3)}, {Q={1}}) (0x0014)
    node = node->getChild(3);
    this->verifyNode(patternTree, node, 3, 14, ItemIDList() << 0 << 1 << 3, QVector<SupportCount>() << 1);

    // Second branch.
    // root -> ({A(0)}, {Q={6}}) (0x0002) -> ({A(0), D(3)}, {Q={3}}) (0x0010)
    node = root->getChild(0)->getChild(3);
    this->verifyNode(patternTree, node, 3, 10, ItemIDList() << 0 << 3, QVector<SupportCount>() << 3);

    // Third branch.
    // root -> ({B(1)}, {Q={5}}) (0x0004)
    node = root->getChild(1);
    this->verifyNode(patternTree, node, 1, 4, ItemIDList() << 1, QVector<SupportCount>() << 5);
    // root -> ({B(1)}, {Q={5}}) (0x0004) -> ({B(1), D(3)}, {Q={1}}) (0x0012)
    node = node->getChild(3);
    this->verifyNode(patternTree, node, 3, 12, ItemIDList() << 1 << 3, QVector<SupportCount>() << 1);

    // Fourth branch.
    // root -> ({C(2)}, {Q={8}}) (0x0001)
    node = root->getChild(2);
    this->verifyNode(patternTree, node, 2, 1, ItemIDList() << 2, QVector<SupportCount>() << 8);
    // root -> ({C(2)}, {Q={8}}) (0x0001) -> ({C(2), A(0)}, {Q={4}}) (0x0003)
    node = node->getChild(0);
    this->verifyNode(patternTree, node, 0, 3, ItemIDList() << 2 << 0, QVector<SupportCount>() << 4);
    // root -> ({C(2)}, {Q={8}}) (0x0001) -> ({C(2), A(0)}, {Q={4}}) (0x0003) -> ({C(2), A(0), B(1)}, {Q={2}}) (0x0007)
    node = node->getChild(1);
    this->verifyNode(patternTree, node, 1, 7, ItemIDList() << 2 << 0 << 1, QVector<SupportCount>() << 2);
    // root -> ({C(2)}, {Q={8}}) (0x0001) -> ({C(2), A(0)}, {Q={4}}) (0x0003) -> ({C(2), A(0), B(1)}, {Q={2}}) (0x0007) -> ({C(2), A(0), B(1), D(3)}, {Q={1}}) (0x0015)
    node = node->getChild(3);
    this->verifyNode(patternTree, node, 3, 15, ItemIDList() << 2 << 0 << 1 << 3, QVector<SupportCount>() << 1);

    // Fifth branch.
    // root -> ({C(2)}, {Q={8}}) (0x0001) -> ({C(2), A(0)}, {Q={4}}) (0x0003) -> ({C(2), A(0), D(3)}, {Q={2}}) (0x0011)
    node = root->getChild(2)->getChild(0)->getChild(3);
    this->verifyNode(patternTree, node, 3, 11, ItemIDList() << 2 << 0 << 3, QVector<SupportCount>() << 2);

    // Sixth branch.
    // root -> ({C(2)}, {Q={8}}) (0x0001) -> ({C(2), B(1)}, {Q={4}}) (0x0005)
    node = root->getChild(2)->getChild(1);
    this->verifyNode(patternTree, node, 1, 5, ItemIDList() << 2 << 1, QVector<SupportCount>() << 4);
    // root -> ({C(2)}, {Q={8}}) (0x0001) -> ({C(2), B(1)}, {Q={4}}) (0x0005) -> ({C(2), B(1), D(3)}, {Q={1}}) (0x0013)
    node = node->getChild(3);
    this->verifyNode(patternTree, node, 3, 13, ItemIDList() << 2 << 1 << 3, QVector<SupportCount>() << 1);

    // Seventh branch.
    // root -> ({C(2)}, {Q={8}}) (0x0001) -> ({C(2), D(3)}, {Q={3}}) (0x0009)
    node = root->getChild(2)->getChild(3);
    this->verifyNode(patternTree, node, 3, 9, ItemIDList() << 2 << 3, QVector<SupportCount>() << 3);

    // Eighth branch.
    // root -> ({C(2)}, {Q={8}}) (0x0001) -> ({C(2), E(4)}, {Q={1}}) (0x0017)
    node = root->getChild(2)->getChild(4);
    this->verifyNode(patternTree, node, 4, 17, ItemIDList() << 2 << 4, QVector<SupportCount>() << 1);

    // Ninth branch.
    // root -> ({D(3)}, {Q={4}}) (0x0008)
    node = root->getChild(3);
    this->verifyNode(patternTree, node, 3, 8, ItemIDList() << 3, QVector<SupportCount>() << 4);

    // Tenth branch.
    // root -> ({E(4)}, {Q={1}}) (0x0016)
    node = root->getChild(4);
    this->verifyNode(patternTree, node, 4, 16, ItemIDList() << 4, QVector<SupportCount>() << 1);

    // Second batch of transactions.
    // Note that there are 22 transactions that include A, 21 that include A
    // and 20 that include D. This means that if not the same f_list is
    // maintained and extended for subsequent batches, this will result in 2
    // new branches being added: A -> C and A -> C -> D, neither of which
    // did previously exist.
    // However, if the same f_list remains in use, the frequent itemsets
    // will be ordered according to the previously used order (i.e. f_list
    // from the first batch of transactions), and then this batch will
    // simply result in updates to the C -> A and C -> A -> D.
    // In conclusion, after this batch of transactions has been processed,
    // the node count of the PatternTree should still equal 17. If it is
    // larger, the f_list has not been used/implemented correctly.
    transactions.clear();
    transactions.append(QStringList() << "A");
    transactions.append(QStringList() << "A");
    transactions.append(QStringList() << "C");
    for (int i = 0; i < 20; i++)
        transactions.append(QStringList() << "C" << "A" << "D");
    fpstream->processBatchTransactions(transactions);
    QCOMPARE(patternTree.getNodeCount(), (unsigned int) 17);

    // Third batch of transactions.
    transactions.clear();
    for (int i = 0; i < 20; i++)
        transactions.append(QStringList() << "A" << "B");
    fpstream->processBatchTransactions(transactions);
    QCOMPARE(patternTree.getNodeCount(), (unsigned int) 17);

    // Fourth batch of transactions.
    transactions.clear();
    for (int i = 0; i < 20; i++)
        transactions.append(QStringList() << "A" << "D");
    fpstream->processBatchTransactions(transactions);

    // Helpful for debugging/expanding this test.
    // Currently, this should match:
    // (NULL)
    // -> ({A(0)}, {Q={20, 20, 22, 6}} (lastUpdate=3)) (0x0002)
    //     -> ({A(0), B(1)}, {Q={0, 20, 0, 3}} (lastUpdate=3)) (0x0006)
    //         -> ({A(0), B(1), D(3)}, {Q={0, 0, 0, 1}} (lastUpdate=3)) (0x0014)
    //     -> ({A(0), D(3)}, {Q={20, 0, 20, 3}} (lastUpdate=3)) (0x0010)
    // -> ({B(1)}, {Q={0, 20, 0, 5}} (lastUpdate=3)) (0x0004)
    //     -> ({B(1), D(3)}, {Q={0, 0, 0, 1}} (lastUpdate=3)) (0x0012)
    // -> ({C(2)}, {Q={0, 0, 21, 8}} (lastUpdate=3)) (0x0001)
    //     -> ({C(2), A(0)}, {Q={0, 0, 20, 4}} (lastUpdate=3)) (0x0003)
    //         -> ({C(2), A(0), B(1)}, {Q={0, 0, 0, 2}} (lastUpdate=3)) (0x0007)
    //             -> ({C(2), A(0), B(1), D(3)}, {Q={0, 0, 0, 1}} (lastUpdate=3)) (0x0015)
    //         -> ({C(2), A(0), D(3)}, {Q={0, 0, 20, 2}} (lastUpdate=3)) (0x0011)
    //     -> ({C(2), B(1)}, {Q={0, 0, 0, 4}} (lastUpdate=3)) (0x0005)
    //         -> ({C(2), B(1), D(3)}, {Q={0, 0, 0, 1}} (lastUpdate=3)) (0x0013)
    //     -> ({C(2), D(3)}, {Q={0, 0, 20, 3}} (lastUpdate=3)) (0x0009)
    //     -> ({C(2), E(4)}, {Q={0, 0, 0, 1}} (lastUpdate=3)) (0x0017)
    // -> ({D(3)}, {Q={20, 0, 20, 4}} (lastUpdate=3)) (0x0008)
    // -> ({E(4)}, {Q={0, 0, 0, 1}} (lastUpdate=3)) (0x0016)
    QCOMPARE(patternTree.getNodeCount(), (unsigned int) 17);
    //qDebug() << fpstream->getPatternTree();

    // Fifth batch of transactions, this will provide the tipping point to
    // fill the first hour bucket.
    transactions.clear();
    transactions.append(QStringList() << "E");
    fpstream->processBatchTransactions(transactions);

    // Automatic tipping point + higher bucket filling is tested in the
    // TiltedTimeWindow unit tests. Here, we're only interested in testing tail
    // pruning.
    // Since we're working with minSupport = 0.4 and maxSupportError = 0.05,
    // that means bucket 4 (the first hour bucket) will be thrown away when
    // the support in bucket 4 is smaller than ceil(0.05 * 73) = 4.
    // The fifth batch of transactions cannot affect the support for the first
    // hour bucket (bucket 4), since it will equal the sum of the first 4
    // quarter buckets, which are displayed above. Based on this, the
    // following buckets should have their first hour bucket tail pruned:
    // - A -> B -> D
    // - B -> D
    // - C -> A -> B
    // - C -> A -> B -> D
    // - C -> B -> D
    // - C -> E
    // - E
    // The edge case can be found in C -> B, where the support equals 4, which
    // makes it just qualified enough to remain.

    // Helpful for debugging/expanding this test.
    // Currently, this should match:
    // (NULL)
    // -> ({A(0)}, {Q={0}, H={68}} (lastUpdate=4)) (0x0002)
    //     -> ({A(0), B(1)}, {Q={0}, H={23}} (lastUpdate=4)) (0x0006)
    //         -> ({A(0), B(1), D(3)}, {Q={0}} (lastUpdate=4)) (0x0014)
    //     -> ({A(0), D(3)}, {Q={0}, H={43}} (lastUpdate=4)) (0x0010)
    // -> ({B(1)}, {Q={0}, H={25}} (lastUpdate=4)) (0x0004)
    //     -> ({B(1), D(3)}, {Q={0}} (lastUpdate=4)) (0x0012)
    // -> ({C(2)}, {Q={0}, H={29}} (lastUpdate=4)) (0x0001)
    //     -> ({C(2), A(0)}, {Q={0}, H={24}} (lastUpdate=4)) (0x0003)
    //         -> ({C(2), A(0), B(1)}, {Q={0}} (lastUpdate=4)) (0x0007)
    //             -> ({C(2), A(0), B(1), D(3)}, {Q={0}} (lastUpdate=4)) (0x0015)
    //         -> ({C(2), A(0), D(3)}, {Q={0}, H={22}} (lastUpdate=4)) (0x0011)
    //     -> ({C(2), B(1)}, {Q={0}, H={4}} (lastUpdate=4)) (0x0005)
    //         -> ({C(2), B(1), D(3)}, {Q={0}} (lastUpdate=4)) (0x0013)
    //     -> ({C(2), D(3)}, {Q={0}, H={23}} (lastUpdate=4)) (0x0009)
    //     -> ({C(2), E(4)}, {Q={0}} (lastUpdate=4)) (0x0017)
    // -> ({D(3)}, {Q={0}, H={44}} (lastUpdate=4)) (0x0008)
    // -> ({E(4)}, {Q={1}} (lastUpdate=4)) (0x0016)
    QCOMPARE(patternTree.getNodeCount(), (unsigned int) 17);
    //qDebug() << fpstream->getPatternTree();

    // Verify the above claims.
    // root -> A -> B -> D
    node = root->getChild(0)->getChild(1)->getChild(3);
    this->verifyNode(patternTree, node, 3, 14, ItemIDList() << 0 << 1 << 3, QVector<SupportCount>() << 0);
    // root -> B -> D
    node = root->getChild(1)->getChild(3);
    this->verifyNode(patternTree, node, 3, 12, ItemIDList() << 1 << 3, QVector<SupportCount>() << 0);
    // root -> C -> A -> B
    node = root->getChild(2)->getChild(0)->getChild(1);
    this->verifyNode(patternTree, node, 1, 7, ItemIDList() << 2 << 0 << 1, QVector<SupportCount>() << 0);
    // root -> C -> A -> B -> D
    node = root->getChild(2)->getChild(0)->getChild(1)->getChild(3);
    this->verifyNode(patternTree, node, 3, 15, ItemIDList() << 2 << 0 << 1 << 3, QVector<SupportCount>() << 0);
    // root -> C -> B -> D
    node = root->getChild(2)->getChild(1)->getChild(3);
    this->verifyNode(patternTree, node, 3, 13, ItemIDList() << 2 << 1 << 3, QVector<SupportCount>() << 0);
    // root -> C -> E
    node = root->getChild(2)->getChild(4);
    this->verifyNode(patternTree, node, 4, 17, ItemIDList() << 2 << 4, QVector<SupportCount>() << 0);
    // root -> E
    node = root->getChild(4);
    this->verifyNode(patternTree, node, 4, 16, ItemIDList() << 4, QVector<SupportCount>() << 1);

    // Sixth batch of transactions, this will provide the tipping point to
    // actually make the previously tail pruned windows be dropped. Except for
    // the "E" pattern, since we've used that in both the previous and current
    // batch as the transaction, hence its tilted time window will not be
    // empty.
    // Hence, six patterns will be dropped from the PatternTree, and the node
    // count of the PatternTree will drop from 17 to 11.
    transactions.clear();
    transactions.append(QStringList() << "E");
    fpstream->processBatchTransactions(transactions);

    // Helpful for debugging/expanding this test.
    // Currently, this should match:
    // (NULL)
    // -> ({A(0)}, {Q={0, 0}, H={68}} (lastUpdate=5)) (0x0002)
    //     -> ({A(0), B(1)}, {Q={0, 0}, H={23}} (lastUpdate=5)) (0x0006)
    //     -> ({A(0), D(3)}, {Q={0, 0}, H={43}} (lastUpdate=5)) (0x0010)
    // -> ({B(1)}, {Q={0, 0}, H={25}} (lastUpdate=5)) (0x0004)
    // -> ({C(2)}, {Q={0, 0}, H={29}} (lastUpdate=5)) (0x0001)
    //     -> ({C(2), A(0)}, {Q={0, 0}, H={24}} (lastUpdate=5)) (0x0003)
    //         -> ({C(2), A(0), D(3)}, {Q={0, 0}, H={22}} (lastUpdate=5)) (0x0011)
    //     -> ({C(2), B(1)}, {Q={0, 0}, H={4}} (lastUpdate=5)) (0x0005)
    //     -> ({C(2), D(3)}, {Q={0, 0}, H={23}} (lastUpdate=5)) (0x0009)
    // -> ({D(3)}, {Q={0, 0}, H={44}} (lastUpdate=5)) (0x0008)
    // -> ({E(4)}, {Q={1, 1}} (lastUpdate=5)) (0x0016)
    QCOMPARE(patternTree.getNodeCount(), (unsigned int) 11);
    //qDebug() << fpstream->getPatternTree();

    delete fpstream;
}

void TestFPStream::verifyNode(const PatternTree & patternTree, const FPNode<TiltedTimeWindow> * const node, ItemID itemID, unsigned int nodeID, const ItemIDList & referencePattern, const QVector<SupportCount> & referenceBuckets) {
    QVERIFY(node != NULL);
    QCOMPARE(node->getItemID(), (ItemID) itemID);
    QCOMPARE(node->getValue().getBuckets(referenceBuckets.size()), referenceBuckets);
    QCOMPARE(node->getNodeID(), nodeID);
    QCOMPARE(PatternTree::getPatternForNode(node), referencePattern);
    QCOMPARE(patternTree.getPatternSupport(referencePattern)->getBuckets(referenceBuckets.size()), referenceBuckets);
}
