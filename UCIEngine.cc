#include <iostream>
#include <exception>

#include "Engine.h"
#include "UCIHandler.h"

int main() {
  try {
    UCIHandler handler(std::cin, std::cout, Engine::LogSection::ALL);
    handler.start();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
  return 0;
}
