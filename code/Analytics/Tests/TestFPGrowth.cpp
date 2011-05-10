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

    FPNode<SupportCount>::resetLastNodeID();
    ItemIDNameHash itemIDNameHash;
    QList<FrequentItemset> frequentItemsets = fpgrowth->mineFrequentItemsets();
    ItemNameIDHash itemNameIDHash;
    FPGrowth * fpgrowth = new FPGrowth(transactions, 0.4 * transactions.size(), &itemIDNameHash, &itemNameIDHash);

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

    // Helpful for debugging/expanding this test.
    // Currently, this should match:
    // (({C(2)}, sup: 8), ({A(0)}, sup: 6), ({C(2), A(0)}, sup: 4), ({B(1)}, sup: 5), ({C(2), B(1)}, sup: 4), ({D(3)}, sup: 4))
    //qDebug() << frequentItemsets;

    // Verify the results.
    QCOMPARE(frequentItemsets, QList<FrequentItemset>() << FrequentItemset(ItemIDList() << 2     , 8)
                                                        << FrequentItemset(ItemIDList() << 0     , 6)
                                                        << FrequentItemset(ItemIDList() << 2 << 0, 4)
                                                        << FrequentItemset(ItemIDList() << 1     , 5)
                                                        << FrequentItemset(ItemIDList() << 2 << 1, 4)
                                                        << FrequentItemset(ItemIDList() << 3     , 4)
    );

    delete fpgrowth;
}

void TestFPGrowth::withConstraints() {
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

    Constraints constraints;
    constraints.addItemConstraint("A", Analytics::CONSTRAINT_POSITIVE_MATCH_ANY);

    FPNode<SupportCount>::resetLastNodeID();
    ItemIDNameHash itemIDNameHash;
    QList<FrequentItemset> frequentItemsets = fpgrowth->mineFrequentItemsets();
    ItemNameIDHash itemNameIDHash;
    FPGrowth * fpgrowth = new FPGrowth(transactions, 0.4 * transactions.size(), &itemIDNameHash, &itemNameIDHash);
    fpgrowth->setConstraints(constraints);

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
    // * frequent itemsets: {{A}, {A, C}}

    // Helpful for debugging/expanding this test.
    // Currently, this should match:
    // (({A(0)}, sup: 6), ({C(2), A(0)}, sup: 4))
    //qDebug() << frequentItemsets;

    // Verify the results.
    QCOMPARE(frequentItemsets, QList<FrequentItemset>() << FrequentItemset(ItemIDList() << 0     , 6)
                                                        << FrequentItemset(ItemIDList() << 2 << 0, 4)
    );

    delete fpgrowth;
}
