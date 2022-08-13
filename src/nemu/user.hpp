#ifndef NEMU_USER_HPP
#define NEMU_USER_HPP

#include "keymap.hpp"
#include "window_info.hpp"

namespace nemu {

struct User {
  Keymap keymap;
  WindowInfo window_info;
};

}  // namespace nemu

namespace sdata {
using namespace nemu;

template<>
struct Serializer<User> : Scheme<User(Keymap, WindowInfo)> {
  Map map(User &user) {
    return Map {
      {"keymap", user.keymap},
      {"window", user.window_info},
    };
  }
};

}  // namespace sdata

#endif
