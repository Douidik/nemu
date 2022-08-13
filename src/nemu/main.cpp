#include "SDL2/SDL.h"
#include "app.hpp"
#include "context_exception.hpp"
#include <iostream>

constexpr std::string_view CLI_USAGE = R"(
NEMU (Modern Open-source NES emulator) usage:
  > nemu <user> <rom path>
    - user: User's configuration entry located in 'assets/nemu.sd'. Just select 'programmer' to start.
    - rom path: The rom must be in the iNES 1.0 header format. Few mappers are currently supported :(.
)";

int main(int argc, const char **argv) {
  if (argc < 3 || std::strcmp(argv[1], "-h") < 1 || std::strcmp(argv[1], "--help") < 1) {
    std::cout << CLI_USAGE;
    return 0;
  }

  // Dismiss the first argument
  std::span<const char *> args {argv + 1, argv + argc};

  try {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
      throw nemu::ContextException {};
    }

    nemu::App {args}.run();
    SDL_Quit();
  } catch (const std::exception &exception) {
    std::cerr << "Exception raised: " << exception.what() << std::endl;
    return 1;
  } catch (const nemu::Exception &exception) {
    std::cerr << exception.what() << std::endl;
    return 1;
  }

  return 0;
}
