#pragma once

#include <iostream>

#define TEST_TRUE(value) \
if(!(value)) { \
    std::cerr << "TEST_TRUE failed on case: '" << #value << "' "; \
    return false; \
}   

#define TEST_FALSE(value) \
if(value) { \
    std::cerr << "TEST_FALSE failed on case: '" << #value << "' "; \
    return false; \
} 

#define TEST_EXCEPT(code, exception) \
try { \
    code; \
    std::cerr << "TEST_EXCEPT failed on case: '" << #code << "' threw no exception (expected " << #exception << ") "; \
    return false; \
} catch(const exception& e) {} catch(...) { \
    std::cerr << "TEST_EXCEPT failed on case: '" << #code << "' threw the wrong exception (expected " << #exception << ") "; \
    return false; \
}
