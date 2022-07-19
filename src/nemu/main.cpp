#include "app.hpp"
#include "exception.hpp"
#include <iostream>

int main(int argc, const char **argv) {
  std::span<const char*> args {argv, argv + argc};
  
  try {
    nemu::App app {args};
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
