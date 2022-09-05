#include "TestReport.h"

bool sample_test();

int main() {
    tests::TestReport report;

    report.add_test("sample_test", sample_test);

    report.run();

    return report.report();
}

bool sample_test() {
    //this test passes

    return true;
}