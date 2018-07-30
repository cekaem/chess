#include <iostream>
#include <exception>

#include "Engine.h"
#include "Logger.h"
#include "UCIHandler.h"

int main() {
  try {
    Logger::getLogger().start(9090, Logger::LogSection::UCI_HANDLER |
                                    Logger::LogSection::ENGINE_MOVE_SEARCHES);
    UCIHandler handler(std::cin, std::cout);
    handler.start();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
  return 0;
}
