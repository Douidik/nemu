#ifndef NEMU_KEYMAP_HPP
#define NEMU_KEYMAP_HPP

#include "exception.hpp"
#include "int.hpp"
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_scancode.h>
#include <sdata.hpp>

namespace nemu {

struct Keymap {
  struct Gamepad {
    int32 a, b, select, start, up, down, left, right;
  } gamepad;

  struct App {
    int32 exit, pause;
  } app;
};

}  // namespace nemu

namespace sdata {
using namespace nemu;

template<>
struct Serializer<Keymap> : Convert<Keymap> {
  void encode(Node &node, const Keymap &keymap) {
    auto to_node = [](uint32 scancode, std::string_view name) -> Node {
      std::string_view keyname = SDL_GetScancodeName(SDL_Scancode(scancode));

      if (keyname.empty()) {
        throw nemu::Exception {"Can't get keyname from #{}", scancode};
      }

      return {name, keyname};
    };

    const auto &gamepad = keymap.gamepad;
    const auto &app = keymap.app;

    node = {
      {
        "gamepad",
        {
          to_node(gamepad.a, "a"),
          to_node(gamepad.b, "b"),
          to_node(gamepad.select, "select"),
          to_node(gamepad.start, "start"),
          to_node(gamepad.up, "up"),
          to_node(gamepad.down, "down"),
          to_node(gamepad.left, "left"),
          to_node(gamepad.right, "right"),
        },
      },

      {
        "app",
        {
          to_node(app.exit, "exit"),
          to_node(app.pause, "pause"),
        },
      }};
  }

  void decode(const Node &node, Keymap &keymap) {
    auto from_node = [](const Node &node, std::string_view name) -> int32 {
      auto keyname = node.at(name).get<std::string_view>();
      uint32 scancode = SDL_GetScancodeFromName(keyname.data());

      if (!scancode) {
        throw nemu::Exception {"Can't get scancode from: '{}'", keyname};
      }

      return scancode;
    };

    const auto &gamepad = node.at("gamepad");
    const auto &app = node.at("app");

    keymap = {
      .gamepad {
        from_node(gamepad, "a"),
        from_node(gamepad, "b"),
        from_node(gamepad, "select"),
        from_node(gamepad, "start"),
        from_node(gamepad, "up"),
        from_node(gamepad, "down"),
        from_node(gamepad, "left"),
        from_node(gamepad, "right"),
      },

      .app {
        from_node(app, "exit"),
        from_node(app, "pause"),
      },
    };
  }
};

}  // namespace sdata

#endif
