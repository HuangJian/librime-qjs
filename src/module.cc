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

static void rime_qjs_initialize() {
  LOG(INFO) << "[qjs] registering components from module 'qjs'.";
  Registry& r = Registry::instance();

  QjsHelper::basePath = string(rime_get_api()->get_user_data_dir()) + "/js";
  auto ctx = QjsHelper::getInstance().getContext();
  init_qjs_types(ctx);

  r.Register("qjs_processor", new QuickJSComponent<QuickJSProcessor, Processor>());
  r.Register("qjs_filter", new QuickJSComponent<QuickJSFilter, Filter>());
  r.Register("qjs_translator", new QuickJSComponent<QuickJSTranslator, Translator>());
}

static void rime_qjs_finalize() {}

RIME_REGISTER_MODULE(qjs)
