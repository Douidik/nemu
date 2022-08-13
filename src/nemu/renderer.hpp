#ifndef NEMU_RENDERER_HPP
#define NEMU_RENDERER_HPP

#include "SDL2/SDL_render.h"
#include "int.hpp"
#include <array>

namespace nemu {

struct Canvas;
struct WindowInfo;
class Window;

class Renderer {
public:
  void setup(Window &window);
  void draw(const WindowInfo &window_info, Canvas &canvas, uint64 fps);
  void close();

private:
  void draw_fps(const WindowInfo &window_info, Canvas &canvas, uint64 fps, uint8 n = 1);
  void draw_nes(const WindowInfo &window_info, Canvas &canvas);

  SDL_Renderer *m_renderer;
  SDL_Texture *m_nes_texture;
};

}  // namespace nemu

#endif
