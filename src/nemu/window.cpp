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

  {
    uint32 x = SDL_WINDOWPOS_CENTERED;
    uint32 y = SDL_WINDOWPOS_CENTERED;
    uint32 w = m_width;
    uint32 h = m_height;

    m_window = SDL_CreateWindow("NEMU", x, y, w, h, 0);
  }

  if (!m_window) {
    throw_context_exception();
  }

  m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);

  if (!m_renderer) {
    throw_context_exception();
  }

  {
    uint32 w = Canvas::W;
    uint32 h = Canvas::H;
    uint32 a = SDL_TEXTUREACCESS_STREAMING;
    uint32 f = SDL_PIXELFORMAT_ARGB8888;

    m_nes_texture = SDL_CreateTexture(m_renderer, f, a, w, h);
  }

  if (!m_nes_texture) {
    throw_context_exception();
  }

  m_keystate = SDL_GetKeyboardState(&m_keycount);
}

void Window::process_events() {
  SDL_Event event {};

  // Map each event to it's callback
  while (SDL_PollEvent(&event) != 0) {
    auto events_cb_iter = m_events_cb.find(event.type);

    if (events_cb_iter != m_events_cb.end()) {
      events_cb_iter->second(event);
    }
  }
}

void Window::process_inputs() {
  for (const auto &[keycode, callbacks] : m_inputs_cb) {
    uint32 scancode = SDL_GetScancodeFromKey(keycode);

    if (scancode < m_keycount) {
      const auto &[on_pressed, on_released] = callbacks;
      m_keystate[scancode] ? on_pressed() : on_released();
    }
  }
}

void Window::map_event(SDL_EventType type, std::function<void(const SDL_Event &)> fn) {
  m_events_cb[type] = fn;
}

void Window::map_input(
  SDL_Keycode code,
  std::function<void()> on_pressed,
  std::function<void()> on_released) {
  m_inputs_cb[code] = {on_pressed, on_released};
}

void Window::draw_canvas(const Canvas &canvas) {
  uint32 *nes_frame;
  int32 nes_frame_pitch;

  int n = SDL_LockTexture(m_nes_texture, nullptr, (void **)&nes_frame, &nes_frame_pitch);

  for (uint16 x = 0; x < Canvas::W; x++) {
    for (uint16 y = 0; y < Canvas::H; y++) {
      // Debug drawing palette
      constexpr std::array<uint8[3], 64> COLORS = {{
        {0x80, 0x80, 0x80}, {0x00, 0x3D, 0xA6}, {0x00, 0x12, 0xB0}, {0x44, 0x00, 0x96},
        {0xA1, 0x00, 0x5E}, {0xC7, 0x00, 0x28}, {0xBA, 0x06, 0x00}, {0x8C, 0x17, 0x00},
        {0x5C, 0x2F, 0x00}, {0x10, 0x45, 0x00}, {0x05, 0x4A, 0x00}, {0x00, 0x47, 0x2E},
        {0x00, 0x41, 0x66}, {0x00, 0x00, 0x00}, {0x05, 0x05, 0x05}, {0x05, 0x05, 0x05},
        {0xC7, 0xC7, 0xC7}, {0x00, 0x77, 0xFF}, {0x21, 0x55, 0xFF}, {0x82, 0x37, 0xFA},
        {0xEB, 0x2F, 0xB5}, {0xFF, 0x29, 0x50}, {0xFF, 0x22, 0x00}, {0xD6, 0x32, 0x00},
        {0xC4, 0x62, 0x00}, {0x35, 0x80, 0x00}, {0x05, 0x8F, 0x00}, {0x00, 0x8A, 0x55},
        {0x00, 0x99, 0xCC}, {0x21, 0x21, 0x21}, {0x09, 0x09, 0x09}, {0x09, 0x09, 0x09},
        {0xFF, 0xFF, 0xFF}, {0x0F, 0xD7, 0xFF}, {0x69, 0xA2, 0xFF}, {0xD4, 0x80, 0xFF},
        {0xFF, 0x45, 0xF3}, {0xFF, 0x61, 0x8B}, {0xFF, 0x88, 0x33}, {0xFF, 0x9C, 0x12},
        {0xFA, 0xBC, 0x20}, {0x9F, 0xE3, 0x0E}, {0x2B, 0xF0, 0x35}, {0x0C, 0xF0, 0xA4},
        {0x05, 0xFB, 0xFF}, {0x5E, 0x5E, 0x5E}, {0x0D, 0x0D, 0x0D}, {0x0D, 0x0D, 0x0D},
        {0xFF, 0xFF, 0xFF}, {0xA6, 0xFC, 0xFF}, {0xB3, 0xEC, 0xFF}, {0xDA, 0xAB, 0xEB},
        {0xFF, 0xA8, 0xF9}, {0xFF, 0xAB, 0xB3}, {0xFF, 0xD2, 0xB0}, {0xFF, 0xEF, 0xA6},
        {0xFF, 0xF7, 0x9C}, {0xD7, 0xE8, 0x95}, {0xA6, 0xED, 0xAF}, {0xA2, 0xF2, 0xDA},
        {0x99, 0xFF, 0xFC}, {0xDD, 0xDD, 0xDD}, {0x11, 0x11, 0x11}, {0x11, 0x11, 0x11},
      }};

      // Free the canvas before being accessed
      nes_frame[y * Canvas::W + x] = {};

      auto [r, g, b] = COLORS[canvas.buffer[x][y]];
      {
        nes_frame[y * Canvas::W + x] |= (b << 8 * 0);
        nes_frame[y * Canvas::W + x] |= (g << 8 * 1);
        nes_frame[y * Canvas::W + x] |= (r << 8 * 2);
        nes_frame[y * Canvas::W + x] |= (0xFF000000);
      }
    }
  }

  SDL_UnlockTexture(m_nes_texture);

  SDL_Rect src_rect {0, 0, Canvas::W, Canvas::H};
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
