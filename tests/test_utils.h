#pragma once

#define TEST_TRUE(value) if(!(value)) return false;
#define TEST_FALSE(value) if(value) return false;

#define TEST_EXCEPT(code, exception) try { code; return false; } catch(const exception& e) {} catch(...) { return false; }