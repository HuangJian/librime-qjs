#include <rime/common.h>
#include <rime/registry.h>
#include <rime_api.h>

#include <quickjs.h>
#include <string>

#include "engines/common.h"
#include "gears/qjs_filter.h"
#include "gears/qjs_processor.h"
#include "gears/qjs_translator.h"
#include "qjs_component.h"

using namespace rime;

template <typename T>
static void setupJsEngine(Registry& r, const std::string& prefix) {
  r.Register(prefix + "_processor", new QuickJSComponent<QuickJSProcessor<T>, Processor, T>());
  r.Register(prefix + "_filter", new QuickJSComponent<QuickJSFilter<T>, Filter, T>());
  r.Register(prefix + "_translator", new QuickJSComponent<QuickJSTranslator<T>, Translator, T>());

  JsEngine<T>::setup();
}

// NOLINTBEGIN(readability-identifier-naming)
static void rime_qjs_initialize() {
  LOG(INFO) << "[qjs] registering components from module 'qjs'.";
  Registry& r = Registry::instance();

  setupJsEngine<QjsValueRAII>(r, "qjs");

#ifdef _ENABLE_JAVASCRIPTCORE
  setupJsEngine<JSValueRef>(r, "jsc");
#else
  // fallback to the quickjs implementation, to share the same Rime schemas across platforms
  setupJsEngine<QjsValueRAII>(r, "jsc");
#endif
}

static void rime_qjs_finalize() {
  JsEngine<QjsValueRAII>::shutdown();

#ifdef _ENABLE_JAVASCRIPTCORE
  JsEngine<JSValueRef>::shutdown();
#endif
}

void rime_require_module_qjs() {}

#ifdef _WIN32

static void __cdecl rime_register_module_qjs(void);
__declspec(allocate(".CRT$XCU")) void(__cdecl* rime_register_module_qjs_)(void) =
    rime_register_module_qjs;
static void __cdecl rime_register_module_qjs(void) {
  static RimeModule module = {0};
  if (!module.data_size) {
    module.data_size = sizeof(RimeModule) - sizeof((module).data_size);
    module.module_name = "qjs";
    module.initialize = rime_qjs_initialize;
    module.finalize = rime_qjs_finalize;
  }
  RimeRegisterModule(&module);
}

#else

static void rime_register_module_qjs() __attribute__((constructor));

static void rime_register_module_qjs() {
  static RimeModule module = {
      .data_size = sizeof(RimeModule) - sizeof((module).data_size),
      .module_name = "qjs",
      .initialize = rime_qjs_initialize,
      .finalize = rime_qjs_finalize,
      .get_api = nullptr,
  };
  RimeRegisterModule(&module);
}
#endif

// NOLINTEND(readability-identifier-naming)
