#ifndef NEMU_WINDOW_HPP
#define NEMU_WINDOW_HPP

#include "ppu/ppu.hpp"
#include <SDL2/SDL.h>
#include <functional>
#include <string_view>
#include <unordered_map>

namespace nemu {

class Window {
public:
  Window(uint32 w, uint32 h, std::string_view name);

  void run_context();
  void process_inputs();
  void process_events();
  void end_context();

  void draw_canvas(const Canvas &canvas);
  void map_event(SDL_EventType type, std::function<void(const SDL_Event &)> fn);
  void map_input(SDL_Keycode code, std::function<void()> fn);

private:
  void throw_context_exception();

  uint32 m_width, m_height;
  std::string_view m_name;
  std::unordered_map<uint32, std::function<void(const SDL_Event &)>> m_events_cb;
  std::unordered_map<uint32, std::function<void()>> m_inputs_cb;

  SDL_Window *m_window;
  SDL_Renderer *m_renderer;
  SDL_Texture *m_nes_texture;
  const uint8 *m_keystate;
  int32 m_keycount;
};

}  // namespace nemu

#endif
