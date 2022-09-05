#include "TestReport.h"

#include <iostream>

void tests::TestReport::add_test(const std::string& name, Test_t to_add) {
    this->tests.push_back(std::make_pair(name, to_add));
}

void tests::TestReport::run() {
    this->passed = 0;
    for(const std::pair<std::string, Test_t>& to_run : this->tests) {
        std::cout << "Running test " << to_run.first << "... ";

        if(to_run.second()) {
            this->passed++;
            std::cout << "passed!\n";
        } else {
            std::cout << "failed!\n";
        }
    }

    std::cout << std::flush;
}

int tests::TestReport::report() {
    std::cout << "Latest test report: "
              << this->tests.size() << " test(s); "
              << this->passed << " passed; "
              << (this->tests.size() - this->passed) << " failed"
              << std::endl;

    return this->tests.size() - this->passed;
}
