#include "window.hpp"
#include "exception.hpp"

namespace nemu {

Window::Window(uint32 w, uint32 h, std::string_view name) :
  m_width {w},
  m_height {h},
  m_name {name} {}

void Window::run_context() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
    throw_context_exception();
  }

  m_window = SDL_CreateWindow("NEMU", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, 0);

  if (!m_window) {
    throw_context_exception();
  }

  m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);

  if (!m_renderer) {
    throw_context_exception();
  }

  uint32 w = Canvas::WIDTH;
  uint32 h = Canvas::HEIGHT;
  uint32 a = SDL_TEXTUREACCESS_STREAMING;
  uint32 f = SDL_PIXELFORMAT_ARGB8888;

  m_nes_texture = SDL_CreateTexture(m_renderer, f, a, w, h);

  if (!m_nes_texture) {
    throw_context_exception();
  }

  m_keystate = SDL_GetKeyboardState(&m_keycount);
}

void Window::process_events() {
  SDL_Event event;

  // Map each event to it's callback
  while (SDL_PollEvent(&event) != 0) {
    auto events_cb_iter = m_events_cb.find(event.type);

    if (events_cb_iter != m_events_cb.end()) {
      events_cb_iter->second(event);
    }
  }
}

void Window::process_inputs() {
  for (int32 scancode = 0; scancode < m_keycount; scancode++) {
    uint32 keycode = SDL_SCANCODE_TO_KEYCODE(scancode);

    if (m_keystate[scancode] && m_inputs_cb.contains(keycode)) {
      m_inputs_cb.at(keycode)();
    }
  }
}

void Window::map_event(SDL_EventType type, std::function<void(const SDL_Event &)> fn) {
  m_events_cb[type] = fn;
}

void Window::map_input(SDL_Keycode code, std::function<void()> fn) {
  m_inputs_cb[code] = fn;
}

void Window::draw_canvas(const Canvas &canvas) {
  uint32 *nes_frame;
  int32 nes_frame_pitch;

  SDL_LockTexture(m_nes_texture, nullptr, (void **)&nes_frame, &nes_frame_pitch);
  
  for (uint16 x = 0; x < Canvas::WIDTH; x++) {
    for (uint16 y = 0; y < Canvas::HEIGHT; y++) {
      constexpr std::array<uint8[3], 4> COLORS = {{
        {200, 200, 200},
        {050, 050, 230},
        {255, 050, 050},
        {000, 000, 000},
      }};

      auto [r, g, b] = COLORS[canvas.buffer[x][y]];

      nes_frame[y * Canvas::WIDTH + x] |= (b << 0x00);
      nes_frame[y * Canvas::WIDTH + x] |= (g << 0x08);
      nes_frame[y * Canvas::WIDTH + x] |= (r << 0x10);
      nes_frame[y * Canvas::WIDTH + x] |= (0xFF << 0x18);
    }
  }

  SDL_UnlockTexture(m_nes_texture);

  SDL_Rect src_rect {0, 0, Canvas::WIDTH, Canvas::HEIGHT};
  SDL_Rect dst_rect {0, 0, (int32)m_width, (int32)m_width};

  SDL_RenderClear(m_renderer);
  SDL_RenderCopy(m_renderer, m_nes_texture, &src_rect, &dst_rect);
  SDL_RenderPresent(m_renderer);
}

void Window::end_context() {
  SDL_DestroyTexture(m_nes_texture);
  SDL_DestroyRenderer(m_renderer);
  SDL_DestroyWindow(m_window);
  SDL_Quit();
}

void Window::throw_context_exception() {
  throw Exception {"SDL exception: '{}'", SDL_GetError()};
}

}  // namespace nemu
