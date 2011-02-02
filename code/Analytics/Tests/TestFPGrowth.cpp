#include "TestFPGrowth.h"
#include <QTime>

void TestFPGrowth::basic() {
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

    FPNode::resetLastNodeID();
    FPGrowth * fpgrowth = new FPGrowth(transactions, 0.4);

    fpgrowth->preprocessingPhase1();
    fpgrowth->preprocessingPhase2();
    QList<ItemList> frequentItemsets = fpgrowth->calculatingPhase1();
    QList<SupportCount> frequentItemsetsSupportCounts = fpgrowth->calculatingPhase2(frequentItemsets);

    // Characteristics about the transactions above, and the found results:
    // * support:
    //   - A: 6
    //   - B: 5
    //   - C: 8
    //   - D: 4
    //   - E: 1
    // * minimum support = 0.4
    // * number of transactions: 10
    // * absolute min support: 4
    // * items qualifying: A, B, C, D
    // * frequent itemsets: {{A}, {B}, {C}, {D}, {C, B}, {C, A}}

    // Verify the results.
    QCOMPARE(frequentItemsets, QList<ItemList>() << (ItemList() << Item(3, 4))
                                                 << (ItemList() << Item(1, 5))
                                                 << (ItemList() << Item(2, 4) << Item(1, 5))
                                                 << (ItemList() << Item(0, 6))
                                                 << (ItemList() << Item(2, 4) << Item(0, 6))
                                                 << (ItemList() << Item(2, 8))
    );
    QCOMPARE(frequentItemsetsSupportCounts, QList<SupportCount>() << 4 << 5 << 4 << 6 << 4 << 8);
}
