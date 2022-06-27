#include "app.hpp"
#include "exception.hpp"
#include <iostream>

int main(int argc, const char **argv) {
  try {
    nemu::App app {std::span(argv, argv + argc)};
    app.run();
  } catch (const std::exception &exception) {
    std::cerr << "Exception raised: " << exception.what() << std::endl;
    return 1;
  } catch (const nemu::Exception &exception) {
    std::cerr << exception.what() << std::endl;
    return 1;
  }

  return 0;
}
