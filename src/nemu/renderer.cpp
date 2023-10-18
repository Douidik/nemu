#include "renderer.hpp"
#include "context_exception.hpp"
#include "digits.hpp"
#include "ppu/ppu.hpp"
#include "window.hpp"
#include <SDL2/SDL.h>

namespace nemu {

constexpr std::array<uint8[3], 65> COLORS = {{
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

  {0xFF, 0xFF, 0xFF},
}};

void Renderer::setup(Window &window) {
  if (!SDL_WasInit(SDL_INIT_VIDEO)) {
    throw Exception {"SDL must initilalize SDL_INIT_VIDEO to setup the renderer"};
  }

  m_renderer = SDL_CreateRenderer(window.native(), -1, SDL_RENDERER_ACCELERATED);

  if (!m_renderer) {
    throw ContextException {};
  }

  {
    uint32 w = Canvas::W;
    uint32 h = Canvas::H;
    uint32 a = SDL_TEXTUREACCESS_STREAMING;
    uint32 f = SDL_PIXELFORMAT_ARGB8888;

    m_nes_texture = SDL_CreateTexture(m_renderer, f, a, w, h);
  }

  if (!m_nes_texture) {
    throw ContextException {};
  }
}

void Renderer::draw(const WindowInfo &window_info, Canvas &canvas, uint64 fps) {
  SDL_RenderClear(m_renderer);
  {
    //draw_fps(window_info, canvas, fps);
    draw_nes(window_info, canvas);

    int32 height = window_info.height;
    int32 width = std::min<int32>(window_info.width, height * (Canvas::W / Canvas::H));
    int32 x = window_info.width / 2 - width / 2;
    int32 y = window_info.height / 2 - height / 2;

    SDL_Rect src_rect {0, 0, Canvas::W, Canvas::H};
    SDL_Rect dst_rect {x, y, width, height};

    SDL_UnlockTexture(m_nes_texture);
    SDL_RenderCopy(m_renderer, m_nes_texture, &src_rect, &dst_rect);
  }
  SDL_RenderPresent(m_renderer);
}

void Renderer::close() {
  SDL_DestroyTexture(m_nes_texture);
  SDL_DestroyRenderer(m_renderer);
}

void Renderer::draw_fps(const WindowInfo &window_info, Canvas &canvas, uint64 fps, uint8 n) {
  const auto &digit = DIGITS[fps % 10];

  for (uint8 c = 0; c < DIGIT_W; c++) {
    for (uint8 r = 0; r < DIGIT_H; r++) {
      uint8 x = c + Canvas::W - (n * DIGIT_OFFSET) - 1;
      uint8 y = r + DIGIT_H;

      if (digit[r][c] != 0) {
        canvas.buffer[x][y] = 64;
      }
    }
  }

  if (fps > 9) {
    draw_fps(window_info, canvas, fps / 10, n + 1);
  }
}

void Renderer::draw_nes(const WindowInfo &window_info, Canvas &canvas) {
  uint32 *nes_frame;
  int32 nes_frame_pitch;

  if (SDL_LockTexture(m_nes_texture, nullptr, (void **)&nes_frame, &nes_frame_pitch) != 0) {
    throw ContextException {};
  }

  for (uint16 x = 0; x < Canvas::W; x++) {
    for (uint16 y = 0; y < Canvas::H; y++) {
      // Free the canvas before drawing
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
}

}  // namespace nemu
