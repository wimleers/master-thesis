#include "TestFPGrowth.h"

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
    QCOMPARE(frequentItemsets, QList<ItemList>() << (ItemList() << Item(3))
                                                 << (ItemList() << Item(1))
                                                 << (ItemList() << Item(2) << Item(1))
                                                 << (ItemList() << Item(0))
                                                 << (ItemList() << Item(2) << Item(0))
                                                 << (ItemList() << Item(2))
    );
}

void TestFPGrowth::withFilter() {
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

    QList<ItemName> filter;
    filter << "A";
    fpgrowth->setFilterItems(filter);
    fpgrowth->preprocessingPhase1();
    fpgrowth->preprocessingPhase2();
    QList<ItemList> frequentItemsets = fpgrowth->calculatingPhase1();

    // Characteristics about the transactions above, and the found results
    // (*after* applying filtering):
    // * support:
    //   - A: 6
    //   - B: 3
    //   - C: 4
    //   - D: 3
    //   - E: 0
    // * minimum support = 0.4
    // * number of transactions: 10
    // * absolute min support: 4
    // * items qualifying: A, C
    // * frequent itemsets: {{A}, {C}, {A, C}}

    // Verify the results.
    QCOMPARE(frequentItemsets, QList<ItemList>() << (ItemList() << Item(0))
                                                 << (ItemList() << Item(2) << Item(0))
                                                 << (ItemList() << Item(2))
    );
}