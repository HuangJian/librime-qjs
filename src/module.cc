#include <rime_api.h>
#include <rime/common.h>
#include <rime/registry.h>

#include "qjs_helper.h"
#include "qjs_component.h"
#include "qjs_types.h"
#include "qjs_filter.h"

using namespace rime;

std::string QjsHelper::basePath = "";

static void rime_qjs_initialize() {
  LOG(INFO) << "[qjs] registering components from module 'qjs'.";
  Registry& r = Registry::instance();

  static JSRuntime *rt = JS_NewRuntime();
  static JSContext *ctx = JS_NewContext(rt);
  init_qjs_types(ctx);

  string jsDirectory = string(rime_get_api()->get_user_data_dir()) + "/js";

  QjsHelper::basePath = jsDirectory + "/";
  QjsHelper::exposeLogToJsConsole(ctx);

  r.Register("qjs_filter", new QuickJSComponent<QuickJSFilter>(ctx, jsDirectory));
}

static void rime_qjs_finalize() {}

RIME_REGISTER_MODULE(qjs)
