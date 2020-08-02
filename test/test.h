#pragma once

#include <assert.h>

#include <functional>
#include <iostream>
#include <sstream>
#include <string>

namespace maf {
namespace test {

using log_callback_t = std::function<void(const std::string&)>;
class call_on_exit {
 public:
  using callback_type = std::function<void()>;
  template <typename Callable>
  call_on_exit(Callable&& f) : f_{std::forward<Callable>(f)} {}

  ~call_on_exit() {
    if (f_) {
      f_();
    }
  }

 private:
  callback_type f_;
};

struct test_cases_sumary_t {
  int total_passed = 0;
  int total_failed = 0;
  static int currentTestCount() {
    static int currentCount = 1;
    return currentCount++;
  }
};

inline test_cases_sumary_t& test_cases_sumary() {
  static test_cases_sumary_t sum;
  return sum;
}

inline log_callback_t& log_callback() {
  static log_callback_t cb = [](const std::string& msg) {
    std::cout << msg << std::endl;
  };
  return cb;
}

struct log_record_t {
  std::ostringstream ss;
  call_on_exit c = [this] { log_callback()(ss.str()); };

 public:
  template <typename T>
  log_record_t& operator<<(const T& val) {
    ss << val;
    return *this;
  }
};

inline log_record_t log_rec() { return log_record_t(); }

inline void init_logging(log_callback_t callback = {}) {
  log_callback();
  if (callback) {
    log_callback() = std::move(callback);
  }
}

inline void init_test_cases() {
  init_logging();
  static struct report {
    ~report() {
      log_rec() << "\n\n-----------------------------------------------";
      log_rec() << "_______________MAF_TEST_REPORT_________________";
      if (test_cases_sumary().total_failed > 0) {
        log_rec() << "^^^^^^^^^^TOTAL TESTS FAILED: "
                  << test_cases_sumary().total_failed << "^^^^^^^";
        log_rec() << "^^^^^^^^^^TOTAL TESTS PASSED: "
                  << test_cases_sumary().total_passed << "^^^^^^^";
        log_rec() << "------------------------------------------------";
        exit(1);
      } else {
        log_rec() << "*************ALL " << test_cases_sumary().total_passed
                  << " TESTS PASSED!*************** ";
      }
      log_rec() << "-----------------------------------------------";
    }
  } report_;
}

}  // namespace test
}  // namespace maf
#define MAF_TEST_CASE_BEGIN(TestCaseName)                                   \
  do {                                                                      \
    using namespace maf::test;                                              \
    int testNumber = test_cases_sumary().currentTestCount();                \
    std::string test_case_name = #TestCaseName "_test";                     \
    int expectation_met = 0;                                                \
    int linefailed = -1;                                                    \
    call_on_exit print_test_result = [testNumber, &test_case_name,          \
                                      &expectation_met, &linefailed] {      \
      assert(expectation_met != 0 &&                                        \
             (test_case_name + " does not have any expectation!").c_str()); \
      if (expectation_met > 0) {                                            \
        test_cases_sumary().total_passed++;                                 \
        log_rec() << "Test case [" << testNumber << "]: " << test_case_name \
                  << " PASSED (" << expectation_met << " expectations)";    \
      } else {                                                              \
        std::cerr << "Test case: " << test_case_name                        \
                  << " FAILED! at line: " << linefailed << std::endl;       \
      }                                                                     \
    };

#define MAF_TEST_CASE_END(...) \
  }                            \
  while (false)                \
    ;

#define MAF_TEST_EXPECT(expected)       \
  if (expected) {                       \
    ++expectation_met;                  \
  } else {                              \
    linefailed = __LINE__;              \
    expectation_met = -1;               \
    test_cases_sumary().total_failed++; \
    break;                              \
  }
