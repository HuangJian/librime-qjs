#include <rime_api.h>
#include <rime/common.h>
#include <rime/registry.h>

#include "qjs_helper.h"
#include "qjs_component.h"
#include "qjs_types.h"
#include "qjs_processor.h"
#include "qjs_filter.h"
#include "qjs_translator.h"

using namespace rime;

// NOLINTBEGIN(readability-identifier-naming)
static void rime_qjs_initialize() {
  LOG(INFO) << "[qjs] registering components from module 'qjs'.";
  Registry& r = Registry::instance();

  QjsHelper::basePath = string(rime_get_api()->get_user_data_dir()) + "/js";
  auto *ctx = QjsHelper::getInstance().getContext();
  initQjsTypes(ctx);

  r.Register("qjs_processor", new QuickJSComponent<QuickJSProcessor, Processor>());
  r.Register("qjs_filter", new QuickJSComponent<QuickJSFilter, Filter>());
  r.Register("qjs_translator", new QuickJSComponent<QuickJSTranslator, Translator>());
}

static void rime_qjs_finalize() {}


void rime_require_module_qjs() {}

static void rime_register_module_qjs() __attribute__((constructor));

static void rime_register_module_qjs() {
  static RimeModule module = {
    .data_size = 0,
   .module_name = nullptr,
  };
  if (module.data_size == 0) {
    ((module).data_size = sizeof(RimeModule) - sizeof((module).data_size));
    module.module_name = "qjs";
    module.initialize = rime_qjs_initialize;
    module.finalize = rime_qjs_finalize;
  }
  RimeRegisterModule(&module);
}

// NOLINTEND(readability-identifier-naming)
