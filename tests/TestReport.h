#pragma once

#include <string>
#include <utility>
#include <vector>
#include <cstddef>

namespace tests {
    typedef bool(*Test_t)();

    class TestReport {
        public:
            TestReport() = default;
        
            void add_test(const std::string& name, Test_t to_add);
            void run();
            int report();

            ~TestReport() = default;
        private:
            size_t passed;
            std::vector<std::pair<std::string, Test_t>> tests;
    };    
}