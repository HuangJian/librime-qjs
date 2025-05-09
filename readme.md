[![Release CI](https://github.com/HuangJian/librime-qjs/actions/workflows/release-ci.yml/badge.svg)](https://github.com/HuangJian/librime-qjs/actions/workflows/release-ci.yml) [English](./doc/readme.en.md) | 中文 | [Plugin Development Guide](./doc/plugin-dev.en.md) | [插件开发指南](./doc/plugin-dev.cn.md)

# librime-qjs

为 Rime 输入法引擎带来浩瀚的 JavaScript 插件生态，以闪电般的速度和羽毛般的轻盈，让输入体验焕然一新！

## 功能特性

- 🔌 为 [Rime 输入法引擎](https://github.com/rime/librime) 带来强大的 JavaScript 插件生态。
  - 🎮 让 JavaScript 尽情发挥，所有 Rime 引擎的精华功能都已为您精心备妥。
  - ✨ 还在为编写代码调试程序绞尽脑汁？ NPM 仓库群星荟萃，应有尽有。
  - 👀 看看我们的实力！[白霜拼音](https://github.com/gaboolic/rime-frost) 的所有 Lua 插件都已完美重写为 [JavaScript 版本](https://github.com/HuangJian/rime-frost/tree/hj/js)。
  - 📝 贴心提供 [JavaScript 类型定义](./contrib/rime.d.ts)，让插件开发体验如丝般顺滑。
  - 🔄 简单灵活的[类型绑定模板](./src/engines/js_macros.h)，让 JavaScript 和 Rime 引擎完美配合。
- 🚀 基于 [QuickJS-NG](https://github.com/quickjs-ng/quickjs) 打造的轻量级 JavaScript 引擎。
  - 💪 畅享最新 ECMAScript 特性：正则表达式、Unicode、ESM、big number，应有尽有！
  - 🚄 运行速度快得惊人：所有插件均毫秒级响应。
  - 🪶 内存占用小得不可思议：<20MB！
  - 🍎 macOS/iOS 尊享极致性能：脚本引擎升级为 JavaScriptCore，速度翻番再翻番！
- 📚 为大型词典量身打造的 LevelDB / Trie 结构。
  - 💥 词典加载快如闪电：11 万词条的[汉译英词典](https://www.mdbg.net/chinese/dictionary?page=cc-cedict)，转为二进制格式后，加载完成仅需 1ms。
  - 🎯 精确查找速如箭矢：11 万词条的汉译英词典，精确查找 200 个汉语词语的英文释义不到 5ms。
  - 🌪️ 前缀搜索迅如疾风：6 万词条的[英译汉词典](https://github.com/skywind3000/ECDICT)，搜索前缀匹配的英文单词及其汉语翻译仅需 1~3ms。
- 🗡️ 所有 JavaScript 插件一次加载到处可用，让输入法会话切换轻松洒脱。
  - 🎉 切换输入法时加载大型插件卡顿严重？现在我们一劳永逸！
  - 🚀 在不同应用间沉浸式写作？插件早已准备就绪！
- 🛡️ 双剑合璧：C++ 和 JavaScript 的单元测试。
  - ✅ 每个 Rime API 都经过严格的 [C++ 测试](./tests/)。
  - 🧪 JavaScript 插件？随心所欲地用 qjs/nodejs/bun/deno [执行测试](https://github.com/HuangJian/rime-frost/tree/hj/js/tests)。

## 文档
- [JavaScript 插件开发指南@quickjs](./doc/plugin-dev.cn.md)
- [JavaScript 插件开发指南@javascriptcore](./doc/plugin-dev-with-jsc.md)
- [libRime-qjs 开发说明@macOS](./doc/build-macos.md)
- [libRime-qjs 开发说明@Windows](./doc/build-windows.md)
- [libRime-qjs 开发说明@Linux](./doc/build-linux.md)
