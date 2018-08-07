/* Component tests for class UCIHandler */

#include <exception>
#include <iostream>
#include <stdexcept>
#include <sstream>

#include "utils/Test.h"
#include "Board.h"
#include "Engine.h"
#include "Field.h"
#include "Figure.h"
#include "UCIHandler.h"


namespace {

TEST_PROCEDURE(test1) {
  TEST_START
  std::stringstream ss;
  UCIHandler handler(ss, ss);
  try {
    handler.handleCommand("invalid_command");
  } catch (const UCIHandler::UnknownCommandException& e) {
    VERIFY_STRINGS_EQUAL("invalid_command", e.command.c_str());
    RETURN
  }
  NOT_REACHED
  TEST_END
}

} // unnamed namespace


int main() {
  try {
    TEST("UCIHandler properly handles unrecognized commands", test1);
    TEST("UCIHandler properly handles command uci", test2);
  } catch (std::exception& except) {
    std::cerr << "Unexpected exception: " << except.what() << std::endl;
     return -1;
  }
  int failed_tests = Test::get_number_of_failed_tests();
  if (failed_tests > 0) {
    std::cout << failed_tests << " test(s) failed." << std::endl;
    return -2;
  }
  std::cout << "All tests passed." << std::endl;
  return 0;
}
