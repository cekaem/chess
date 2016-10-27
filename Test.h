#ifndef TEST_H
#define TEST_H

#include <iostream>
#include <sstream>
#include <utility>

namespace {

template <typename T1, typename T2>
std::ostream& operator<<(std::ostream& ostr, const std::pair<T1, T2>& pair) {
  ostr << "(" << pair.first << ", " << pair.second << ")";
  return ostr;
}

}  // unnamed namespace


class Test {
 public:
  struct TestFailedException {};
  class EndTestException {};

  enum TestResult {
    TEST_OK,
    TEST_FAILED
  };

  Test() {
    error_line_ = 0;
    error_message_.clear();
  }

  static void Check(const char* name, TestResult is_ok) {
    std::cout << name << ": ";
    if (is_ok == TEST_OK) {
      std::cout << "OK" << std::endl;
    } else {
      ++number_of_failed_tests_;
      std::cout << "FAILED (line: " << error_line_ << ")" << std::endl;
      if (!error_message_.empty()) {
        std::cout << "Error message: " << error_message_ << std::endl;
      }
    }
    std::cout << std::endl;
  }

  static void SetErrorMessage(const std::string& error_message) {
    error_message_ = error_message;
  }

  static int get_number_of_failed_tests() {
    return number_of_failed_tests_;
  }

  void Verify(bool expr, int line) {
    if (!expr) {
      error_line_ = line;
      throw TestFailedException();
    }
  }

  static int error_line_;
  static std::string error_message_;
  static int number_of_failed_tests_;
};

int Test::error_line_ = 0;
std::string Test::error_message_;
int Test::number_of_failed_tests_ = 0;

template <typename T>
void VerifyIsEqual(const T& expr1, const T& expr2, int line) {
  if (expr1 != expr2) {
    Test::error_line_ = line;
    std::stringstream ss;
    ss << expr1 << " vs " << expr2;
    Test::SetErrorMessage(ss.str());
    throw Test::TestFailedException();
  }
}

template <typename T>
void VerifyIsNull(const T* ptr1, int line) {
  if (ptr1 != nullptr) {
    Test::error_line_ = line;
    std::stringstream ss;
    ss << "Pointer is not null: " << ptr1;
    Test::SetErrorMessage(ss.str());
    throw Test::TestFailedException();
  }
}


#define TEST_PROCEDURE(T) Test::TestResult T()

#define TEST(str, fun) Test::Check(str, fun())

#define TEST_START \
  try { \
    Test test;

#define VERIFY(expr) test.Verify(expr, __LINE__)
#define VERIFY_IS_EQUAL(expr1, expr2) VerifyIsEqual(expr1, expr2, __LINE__)
#define VERIFY_IS_NULL(ptr) VerifyIsNull(ptr, __LINE__)
#define VERIFY_IS_ZERO(expr) VERIFY_IS_EQUAL(expr, 0)

#define TEST_END \
  }\
  catch (Test::TestFailedException& except) {\
    return Test::TEST_FAILED;\
  }\
  catch (Test::EndTestException& e) {\
  }\
  return Test::TEST_OK;

#define RETURN throw Test::EndTestException();

#define NOT_REACHED VERIFY(1 == 0);

#endif // TEST_H
