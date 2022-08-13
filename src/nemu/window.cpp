#include "window.hpp"
#include "SDL2/SDL.h"
#include "app.hpp"
#include "context_exception.hpp"

namespace nemu {

Window::Window(WindowInfo &info) : m_info {info} {}

void Window::setup() {
  uint32 x = SDL_WINDOWPOS_CENTERED;
  uint32 y = SDL_WINDOWPOS_CENTERED;
  uint32 w = m_info.width;
  uint32 h = m_info.height;
  uint32 f = m_info.options;

  m_window = SDL_CreateWindow("NEMU", x, y, w, h, f | SDL_WINDOW_RESIZABLE);
}

void Window::update(State &state) {
  SDL_Event event {};

  while (SDL_PollEvent(&event) != 0) {
    switch (event.type) {
    case SDL_QUIT: {
      state = State::EXIT;
    } break;

    case SDL_WINDOWEVENT: {
      SDL_WindowEvent window_event = event.window;

      switch (window_event.event) {
      case SDL_WINDOWEVENT_RESIZED:
      case SDL_WINDOWEVENT_SIZE_CHANGED: {
        m_info.width = window_event.data1;
        m_info.height = window_event.data2;
      } break;

      default: break;
      }
    }

    default: break;
    }
  }
}

void Window::close() {
  SDL_DestroyWindow(m_window);
}

}  // namespace nemu
