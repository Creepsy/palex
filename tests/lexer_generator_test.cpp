#include "TestReport.h"
#include "test_utils.h"

int main() {
    tests::TestReport report;

    report.run();

    return report.report();
}