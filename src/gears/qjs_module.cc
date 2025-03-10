#include "qjs_module.h"
#include "qjs_environment.h"
#include "jsvalue_raii.h"

namespace rime {

QjsModule::QjsModule(const std::string& nameSpace,
                     Engine* engine,
                     const char* mainFuncName): namespace_(nameSpace) {
  auto *ctx = QjsHelper::getInstance().getContext();
  std::string fileName = nameSpace + ".js";
  JSValueRAII moduleNamespace(QjsHelper::loadJsModuleToNamespace(ctx, fileName.c_str()));
  if (JS_IsUndefined(moduleNamespace) || JS_IsException(moduleNamespace)) {
    LOG(ERROR) << "[qjs] Failed to load " << fileName;
    return;
  }

  environment_ = QjsEnvironment::create(ctx, engine, nameSpace);

  JSValueRAII initFunc(JS_GetPropertyStr(ctx, moduleNamespace, "init"));
  if (!JS_IsUndefined(initFunc)) {
    JSValueRAII initResult(JS_Call(ctx, initFunc, JS_UNDEFINED, 1, environment_.getPtr()));
    if (JS_IsException(initResult)) {
      LOG(ERROR) << "[qjs] Error running the init function in " << fileName;
      return;
    }
    DLOG(INFO) << "[qjs]  init function executed successfully in " << fileName;

  }

  finitFunc_ = JSValueRAII(JS_GetPropertyStr(ctx, moduleNamespace, "finit"));

  mainFunc_ = JSValueRAII(JS_GetPropertyStr(ctx, moduleNamespace, mainFuncName));
  if (JS_IsUndefined(mainFunc_)) {
    LOG(ERROR) << "[qjs] No `" << mainFuncName << "` function exported in " << fileName;
    return;
  }

  isLoaded_ = true;
}

QjsModule::~QjsModule() {
  if (JS_IsUndefined(finitFunc_)) {
    DLOG(INFO) << "[qjs] ~" << namespace_ << " no `finit` function exported.";
  } else {
    DLOG(INFO) << "[qjs] running the finit function";
    auto *ctx = QjsHelper::getInstance().getContext();
    JSValueRAII finitResult(JS_Call(ctx, finitFunc_, JS_UNDEFINED, 1, environment_.getPtr()));
    if (JS_IsException(finitResult)) {
      LOG(ERROR) << "[qjs] ~" << namespace_ << " Error running the finit function.";
    }
  }
}

} // namespace rime
