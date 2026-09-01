#include <iostream>
#include <vector>
#include <functional>
#include <string>

// Lightweight test framework harness
struct TestCase {
    std::string name;
    std::function<bool()> testFunc;
};

std::vector<TestCase>& getRegisteredTests() {
    static std::vector<TestCase> tests;
    return tests;
}

void registerTest(const std::string& name, std::function<bool()> func) {
    getRegisteredTests().push_back({name, func});
}

#define TEST_CASE(name) \
    bool test_##name(); \
    struct Register_##name { \
        Register_##name() { registerTest(#name, test_##name); } \
    } reg_##name; \
    bool test_##name()

#define EXPECT_TRUE(cond) if (!(cond)) { std::cerr << "  FAIL: " << #cond << " at line " << __LINE__ << "\n"; return false; }
#define EXPECT_NEAR(a, b, eps) if (std::abs((a) - (b)) > (eps)) { std::cerr << "  FAIL: " << #a << " (" << (a) << ") != " << #b << " (" << (b) << ") at line " << __LINE__ << "\n"; return false; }

int main() {
    std::cout << "========================================================================\n";
    std::cout << "                  AMPLIFY AUTOMATED TEST SUITE                          \n";
    std::cout << "========================================================================\n";

    size_t passed = 0;
    size_t failed = 0;

    for (const auto& tc : getRegisteredTests()) {
        std::cout << "[RUN ] " << tc.name << "... ";
        try {
            if (tc.testFunc()) {
                std::cout << "PASSED\n";
                passed++;
            } else {
                std::cout << "FAILED\n";
                failed++;
            }
        } catch (const std::exception& e) {
            std::cout << "FAILED with exception: " << e.what() << "\n";
            failed++;
        } catch (...) {
            std::cout << "FAILED with unknown exception\n";
            failed++;
        }
    }

    std::cout << "========================================================================\n";
    std::cout << "Total Tests: " << (passed + failed) << " | Passed: " << passed << " | Failed: " << failed << "\n";
    std::cout << "========================================================================\n";

    return (failed == 0) ? 0 : 1;
}
