#pragma once

#include <iostream>

#define TEST_TRUE(value) \
if(!(value)) { \
    std::cerr << "[Ln " << __LINE__ << "] TEST_TRUE failed on case: '" << #value << "' "; \
    return 1; \
}   

#define TEST_FALSE(value) \
if(value) { \
    std::cerr << "[Ln " << __LINE__ << "] TEST_FALSE failed on case: '" << #value << "' "; \
    return 1; \
} 

#define TEST_EXCEPT(code, exception) \
try { \
    code; \
    std::cerr << "[Ln " << __LINE__ << "] TEST_EXCEPT failed on case: '" << #code << "' threw no exception (expected " << #exception << ") "; \
    return 1; \
} catch(const exception& e) {} catch(...) { \
    std::cerr << "[Ln " << __LINE__ << "] TEST_EXCEPT failed on case: '" << #code << "' threw the wrong exception (expected " << #exception << ") "; \
    return 1; \
}
