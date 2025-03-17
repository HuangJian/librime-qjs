# librime-qjs   [中文](./doc/readme-cn.md)

Bring a fresh JavaScript plugin ecosystem to the Rime Input Method Engine, delivering lightning-fast speed and feather-light performance for a revolutionary input experience!

## Features

- 🔌 Powerful JavaScript plugin ecosystem for the [Rime Input Method Engine](https://github.com/rime/librime).
  - 🎮 Unleash the full potential of JavaScript with all essential Rime engine features at your fingertips.
  - ✨ See our capabilities in action! All Lua plugins from [Frost Pinyin](https://github.com/gaboolic/rime-frost) have been perfectly rewritten in [JavaScript](https://github.com/HuangJian/rime-frost/tree/hj/js).
  - 📝 Smooth plugin development with comprehensive [JavaScript type definitions](./contrib/rime.d.ts).
  - 🔄 Simple and flexible [type binding templates](./src/helpers/qjs_macros.h) for seamless JavaScript and Rime engine integration.
- 🚀 Lightweight JavaScript engine powered by [QuickJS-NG](https://github.com/quickjs-ng/quickjs).
  - 💪 Enjoy the latest ECMAScript features: regular expressions, Unicode, ESM, big numbers, and more!
  - 🚄 Blazing-fast performance: all plugins respond within milliseconds.
  - 🪶 Incredibly small memory footprint: <20MB!
- 📚 Custom-built Trie structure for large dictionaries.
  - 💥 Lightning-fast dictionary loading: 110,000-entry [Chinese-English dictionary](https://www.mdbg.net/chinese/dictionary?page=cc-cedict) loads in just 20ms after binary conversion.
  - 🎯 Swift exact lookups: finding English definitions for 200 Chinese words in under 5ms.
  - 🌪️ Rapid prefix search: searching English words with Chinese translations in a 60,000-entry [English-Chinese dictionary](https://github.com/skywind3000/ECDICT) takes only 1-3ms.
- 🗡️ Share JavaScript plugins across all Rime sessions for seamless transitions.
  - 🎉 No more lag when switching input methods with large plugins - solved once and for all!
  - 🚀 Ready for immersive writing across different applications!
- 🛡️ Comprehensive testing with both C++ and JavaScript.
  - ✅ Every Rime API thoroughly tested with [C++ tests](./tests/).
  - 🧪 JavaScript plugins? Test freely with qjs/nodejs/bun/deno using our [test suite](https://github.com/HuangJian/rime-frost/tree/hj/js/tests).

## TODO

- [ ] Multiple Platform Support
  - [x] macOS
  - [ ] Windows
  - [x] Linux
  - [ ] Android (PR Welcome)
  - [ ] iOS (PR Welcome)
- [ ] More JavaScript Engine Support (PR Welcome)
  - [ ] JavaScriptCore on macOS/iOS
    - Built-in on macOS/iOS systems
    - Most performant JavaScript engine
  - [ ] V8 engine support via nodejs/deno

## Installation

### ArchLinux

Install from AUR

```shell
paru -S librime-qjs-git
```

### NixOS

Install from NUR

```nix
overlays = [
  ...
  (_final: prev: {
    librime = prev.librime.override {
      plugins = with prev; [
        nur.repos.xyenon.librime-qjs
        librime-lua
        librime-octagram
      ];
    };
  })
  ...
];
```

## Development

- [build on macOS](./doc/build-macos.md)
- [build on Linux](./doc/build-linux.md)
