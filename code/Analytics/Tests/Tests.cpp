#include "TestFPTree.h"
#include "TestFPGrowth.h"

int main() {
    TestFPTree FPTree;
    QTest::qExec(&FPTree);

    TestFPGrowth FPGrowth;
    QTest::qExec(&FPGrowth);

    return 0;
}
