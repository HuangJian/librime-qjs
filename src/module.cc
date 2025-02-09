//
// Copyright RIME Developers
// Distributed under the BSD License
//
// 2014-01-04 GONG Chen <chen.sst@gmail.com>
//

#include <rime_api.h>
#include <rime/common.h>
#include <rime/registry.h>

#include "qjs_component.h"
#include "qjs_types.h"
#include "qjs_filter.h"

using namespace rime;

static void rime_qjs_initialize() {
  LOG(INFO) << "registering components from module 'qjs'.";
  Registry& r = Registry::instance();

  static JSRuntime *rt = JS_NewRuntime();
  static JSContext *ctx = JS_NewContext(rt);
  init_qjs_types(ctx);

  r.Register("qjs_filter", new QuickJSComponent<QuickJSFilter>(rt, ctx));
}

static void rime_qjs_finalize() {}

RIME_REGISTER_MODULE(qjs)
