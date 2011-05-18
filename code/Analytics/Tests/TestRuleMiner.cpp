#include "TestRuleMiner.h"

void TestRuleMiner::basic() {
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

    FPNode<SupportCount>::resetLastNodeID();
    QList<FrequentItemset> frequentItemsets = fpgrowth->mineFrequentItemsets();
    ItemIDNameHash itemIDNameHash;
    ItemNameIDHash itemNameIDHash;
    ItemIDList sortedFrequentItemIDs;
    FPGrowth * fpgrowth = new FPGrowth(transactions, 0.4 * transactions.size(), &itemIDNameHash, &itemNameIDHash, &sortedFrequentItemIDs);

    QList<AssociationRule> associationRules = RuleMiner::mineAssociationRules(frequentItemsets, 0.8, constraints, fpgrowth);

    // Helpful for debugging/expanding this test.
    // Currently, this should match:
    // ({B(1)} => {C(2)} (conf=0.8))
    //qDebug() << associationRules;

    // Verify the results.
    QCOMPARE(associationRules.size(), 1);
    QCOMPARE(associationRules[0].antecedent, (ItemIDList() << 1));
    QCOMPARE(associationRules[0].consequent, (ItemIDList() << 2));
    QCOMPARE(associationRules[0].confidence, (float) 0.8);

    delete fpgrowth;
}
