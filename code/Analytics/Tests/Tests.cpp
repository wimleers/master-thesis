#include "TestFPTree.h"
#include "TestFPGrowth.h"
#include "TestRuleMiner.h"

int main() {
    TestFPTree FPTree;
    QTest::qExec(&FPTree);

    TestFPGrowth FPGrowth;
    QTest::qExec(&FPGrowth);

    TestRuleMiner ruleMiner;
    QTest::qExec(&ruleMiner);

    return 0;
}
