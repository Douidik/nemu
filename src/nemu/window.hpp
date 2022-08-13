#ifndef NEMU_WINDOW_HPP
#define NEMU_WINDOW_HPP

#include "SDL2/SDL_video.h"
#include "window_info.hpp"

namespace nemu {

enum class State : uint32;

class Window {
public:
  Window(WindowInfo &info);

  void setup();
  void update(State &state);
  void close();

  inline SDL_Window *native() {
    return m_window;
  }

private:
  SDL_Window *m_window;
  WindowInfo &m_info;
};

}  // namespace nemu

#endif
