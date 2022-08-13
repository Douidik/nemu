#ifndef NEMU_CONTEXT_EXCEPTION_HPP
#define NEMU_CONTEXT_EXCEPTION_HPP

#include "exception.hpp"
#include <SDL2/SDL_error.h>

namespace nemu {

struct ContextException : public Exception {
  ContextException() : Exception {"SDL exception: '{}'", SDL_GetError()} {}
};

}  // namespace nemu

#endif
