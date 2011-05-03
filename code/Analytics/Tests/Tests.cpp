#include "TestFPTree.h"
#include "TestFPGrowth.h"
#include "TestRuleMiner.h"
#include "TestTiltedTimeWindow.h"

int main() {
    TestFPTree FPTree;
    QTest::qExec(&FPTree);

    TestFPGrowth FPGrowth;
    QTest::qExec(&FPGrowth);

    TestRuleMiner ruleMiner;
    QTest::qExec(&ruleMiner);

    // FP-Stream related classes & tests.
    TestTiltedTimeWindow tiltedTimeWindow;
    QTest::qExec(&tiltedTimeWindow);

    return 0;
}
