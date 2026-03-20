#include "quickjs_code_loader.h"

#include <glog/logging.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <quickjs.h>
#include "engines/quickjs/qjs_value_raii.h"

#include "engines/jscode_utils.h"
#include "patch/quickjs/node_module_loader.h"

void QuickJSCodeLoader::logJsError(JSContext* ctx, const char* prefix, const char* file, int line) {
  if (!JS_HasException(ctx)) {
    google::LogMessage(file, line, google::GLOG_ERROR).stream()
        << "[qjs] the latest exception might be already processed.";
    return;
  }

  const QjsValueRAII exception(ctx, JS_GetException(ctx));
  const QjsCStringRAII message(ctx, JS_ToCString(ctx, exception));
  google::LogMessage(file, line, google::GLOG_ERROR).stream()
      << "[qjs]" << prefix << ' ' << (message.get() ? message.get() : "");

  const QjsValueRAII stack(ctx, JS_GetPropertyStr(ctx, exception, "stack"));
  const QjsCStringRAII stackTrace(ctx, JS_ToCString(ctx, stack));
  if (stackTrace.get() != nullptr && *stackTrace.get() != '\0') {
    google::LogMessage(file, line, google::GLOG_ERROR).stream()
        << "[qjs] JS stack trace: " << stackTrace.get();
  } else {
    google::LogMessage(file, line, google::GLOG_ERROR).stream() << "[qjs] JS stack trace is null.";
  }
}

JSValue QuickJSCodeLoader::loadJsModuleToNamespace(JSContext* ctx, const char* moduleName) {
  const JSValue funcObj = loadJsModule(ctx, moduleName);
  if (JS_IsException(funcObj)) {
    logJsError(ctx, "Failed to load js module:", __FILE__, __LINE__);
    return funcObj;
  }

  auto* md = reinterpret_cast<JSModuleDef*>(JS_VALUE_GET_PTR(funcObj));
  const QjsValueRAII evalResult(ctx, JS_EvalFunction(ctx, funcObj));
  if (JS_IsException(evalResult)) {
    logJsError(ctx, "Failed to evaluate the module:", __FILE__, __LINE__);
    return evalResult.release();
  }

  return JS_GetModuleNamespace(ctx, md);
}

JSValue QuickJSCodeLoader::loadJsModuleToGlobalThis(JSContext* ctx, const char* moduleName) {
  char* jsCode = readJsCode(ctx, moduleName);
  std::string jsCodeStr(jsCode);
  // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
  free(jsCode);
  return JS_Eval(ctx, jsCodeStr.c_str(), jsCodeStr.size(), moduleName, JS_EVAL_TYPE_MODULE);
}

JSValue QuickJSCodeLoader::createInstanceOfEsmBundledModule(JSContext* ctx,
                                                            const std::string& moduleName,
                                                            std::vector<JSValue>& args,
                                                            const std::string& mainFuncName) {
  QjsValueRAII module(ctx, loadJsModuleToNamespace(ctx, moduleName.c_str()));
  if (JS_IsException(module)) {
    logJsError(ctx, "Failed to load js module:", __FILE__, __LINE__);
    return module.release();
  }

  const QjsValueRAII jsClass(
      ctx, getExportedClassHavingMethodNameInModule(ctx, module, mainFuncName.c_str()));

  if (JS_IsException(jsClass)) {
    const std::string message =
        "No exported class having a `" + mainFuncName + "` function in " + moduleName;
    logJsError(ctx, message.c_str(), __FILE__, __LINE__);
    return JS_ThrowPlainError(ctx, "%s", message.c_str());
  }

  const QjsValueRAII constructor(ctx, getMethodByNameInClass(ctx, jsClass, "constructor"));
  if (JS_IsException(constructor)) {
    const std::string message = "No `constructor` function in " + moduleName;
    logJsError(ctx, message.c_str(), __FILE__, __LINE__);
    return JS_ThrowPlainError(ctx, "%s", message.c_str());
  }

  const int argc = static_cast<int>(args.size());
  const QjsValueRAII instance(ctx, JS_CallConstructor(ctx, constructor, argc, args.data()));

  if (JS_IsException(instance)) {
    const std::string message = "Failed to create an instance of " + moduleName;
    logJsError(ctx, message.c_str(), __FILE__, __LINE__);
    return JS_ThrowPlainError(ctx, "%s", message.c_str());
  }

  DLOG(INFO) << "[qjs] Created an instance of " << moduleName;
  return instance.release();
}

JSValue QuickJSCodeLoader::createInstanceOfIifeBundledModule(JSContext* ctx,
                                                             const std::string& baseFolderPath,
                                                             const std::string& moduleName,
                                                             const std::vector<JSValue>& args) {
  const auto* msg = "Memory leaks when creating an module instance inside the IIFE-format code.";
  LOG(ERROR) << "[qjs] " << msg;
  throw std::runtime_error(msg);
  // Indirect leak of 48 byte(s) in 1 object(s) allocated from:
  // #0 0x00010874e457 in realloc+0x87 (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x58457)
  // #1 0x000107d29c61 in js_realloc_rt+0x51 (librime-qjs-tests:x86_64+0x1000f0c61)
  // #2 0x000107d6369e in add_property+0x10e (librime-qjs-tests:x86_64+0x10012a69e)
  // #3 0x000107d400f5 in JS_CreateProperty+0xd5 (librime-qjs-tests:x86_64+0x1001070f5)
  // #4 0x000107d4aa3d in JS_CallInternal+0x720d (librime-qjs-tests:x86_64+0x100111a3d)
  // #5 0x000107d454f4 in JS_CallInternal+0x1cc4 (librime-qjs-tests:x86_64+0x10010c4f4)
  // #6 0x000107d88e2d in js_async_function_resume+0x8d (librime-qjs-tests:x86_64+0x10014fe2d)
  // #7 0x000107d5aee8 in js_async_function_call+0x128 (librime-qjs-tests:x86_64+0x100121ee8)
  // #8 0x000107d71f62 in js_execute_sync_module+0x62 (librime-qjs-tests:x86_64+0x100138f62)
  // #9 0x000107d71cbf in js_inner_module_evaluation+0x2df (librime-qjs-tests:x86_64+0x100138cbf)
  // #10 0x000107d4fde1 in JS_EvalFunctionInternal+0x191 (librime-qjs-tests:x86_64+0x100116de1)
  // #11 0x000107d5d078 in __JS_EvalInternal+0xa88 (librime-qjs-tests:x86_64+0x100124078)
  // #12 0x000107d50127 in JS_EvalThis2+0xf7 (librime-qjs-tests:x86_64+0x100117127)
  // #13 0x000107d502a1 in JS_Eval+0x41 (librime-qjs-tests:x86_64+0x1001172a1)
  // #14 0x000107cf8198 in QuickJSCodeLoader::createInstanceOfIifeBundledModule(JSContext*, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > const&, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > const&, std::__1::vector<JSValue, std::__1::allocator<JSValue> > const&) quickjs_code_loader.cc:147
  // #15 0x000107d1d9f4 in QuickJsEngineImpl::createInstanceOfModule(char const*, std::__1::vector<JSValue, std::__1::allocator<JSValue> >&, std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > const&) const quickjs_engine_impl.cc:119

  // file lookup order: ./dist/module.iife.js > ./dist/module.js > ./module.iife.js > ./module.js
  std::vector<std::string> fileNames = {"dist/" + moduleName + ".iife.js",
                                        "dist/" + moduleName + ".js", moduleName + ".iife.js",
                                        moduleName + ".js"};

  for (const auto& fileName : fileNames) {
    std::filesystem::path filePath = std::filesystem::path(baseFolderPath) / fileName;
    if (!std::filesystem::exists(filePath)) {
      continue;
    }

    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    auto source = buffer.str();
    removeExportStatementsInPlace(source);

    // assign the variables to globalThis with unique names, to avoid name conflicts between modules.
    std::string flatNamespace = moduleName;
    std::replace(flatNamespace.begin(), flatNamespace.end(), '.', '_');
    std::replace(flatNamespace.begin(), flatNamespace.end(), '-', '_');
    auto now = std::chrono::system_clock::now();
    auto tick = std::to_string(
        std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count());
    flatNamespace += "_" + tick;
    std::string instanceName = flatNamespace + "_instance";

    auto globalThis = JS_GetGlobalObject(ctx);
    std::vector<std::string> argumentNames(args.size());
    for (size_t i = 0; i < args.size(); i++) {
      argumentNames[i] = flatNamespace + "_arg" + std::to_string(i);
      JS_SetPropertyStr(ctx, globalThis, argumentNames[i].c_str(), args[i]);
    }
    replaceNewClassInstanceStatementInPlace(source, instanceName, argumentNames);

    auto evalResult =
        JS_Eval(ctx, source.c_str(), source.size(), moduleName.c_str(), JS_EVAL_TYPE_MODULE);
    if (JS_IsException(evalResult)) {
      std::string message = "Failed to evaluate script: " + fileName;
      logJsError(ctx, message.c_str(), __FILE__, __LINE__);
      return evalResult;
    }

    return JS_GetPropertyStr(ctx, globalThis, instanceName.c_str());
  }

  std::string message = "Failed to create an instance of the module: " + moduleName;
  LOG(ERROR) << "[jsc] " << message;
  return JS_ThrowPlainError(ctx, "%s", message.c_str());
}

JSValue QuickJSCodeLoader::getMethodByNameInClass(JSContext* ctx,
                                                  const JSValue classObj,
                                                  const char* methodName) {
  const QjsValueRAII proto(ctx, JS_GetPropertyStr(ctx, classObj, "prototype"));
  if (JS_IsException(proto)) {
    return JS_UNDEFINED;
  }

  const QjsValueRAII method(ctx, JS_GetPropertyStr(ctx, proto, methodName));
  if (JS_IsException(method) || !JS_IsFunction(ctx, method)) {
    return JS_UNDEFINED;
  }

  return method.release();
}

JSValue QuickJSCodeLoader::getExportedClassByNameInModule(JSContext* ctx,
                                                          const JSValue moduleObj,
                                                          const char* className) {
  JSPropertyEnum* props = nullptr;
  uint32_t propCount = 0;  // Get all enumerable properties from namespace
  if (constexpr int flags = JS_GPN_STRING_MASK | JS_GPN_SYMBOL_MASK | JS_GPN_ENUM_ONLY;
      JS_GetOwnPropertyNames(ctx, &props, &propCount, moduleObj, flags) == 0) {
    const size_t n = strlen(className);
    for (uint32_t i = 0; i < propCount; i++) {
      const QjsValueRAII propVal(ctx, JS_GetProperty(ctx, moduleObj, props[i].atom));
      const QjsCStringRAII propName(ctx, JS_AtomToCString(ctx, props[i].atom));
      const bool isMatched = !JS_IsException(propVal) && strncmp(propName, className, n) == 0;

      if (isMatched) {
        for (uint32_t j = i; j < propCount; j++) {
          JS_FreeAtom(ctx, props[j].atom);
        }
        js_free(ctx, props);
        return propVal.release();
      }
      JS_FreeAtom(ctx, props[i].atom);
    }
    js_free(ctx, props);
  }
  return JS_UNDEFINED;
}

JSValue QuickJSCodeLoader::getExportedClassHavingMethodNameInModule(JSContext* ctx,
                                                                    const JSValue moduleObj,
                                                                    const char* methodName) {
  JSPropertyEnum* props = nullptr;
  uint32_t propCount = 0;  // Get all enumerable properties from namespace
  if (constexpr int flags = JS_GPN_STRING_MASK | JS_GPN_SYMBOL_MASK | JS_GPN_ENUM_ONLY;
      JS_GetOwnPropertyNames(ctx, &props, &propCount, moduleObj, flags) == 0) {
    for (uint32_t i = 0; i < propCount; i++) {
      const QjsValueRAII propVal(ctx, JS_GetProperty(ctx, moduleObj, props[i].atom));
      const QjsCStringRAII propName(ctx, JS_AtomToCString(ctx, props[i].atom));

      bool found = false;
      if (JS_IsObject(propVal)) {
        const QjsValueRAII method(ctx, getMethodByNameInClass(ctx, propVal, methodName));
        found = !JS_IsException(method) && !JS_IsUndefined(method) && JS_IsFunction(ctx, method);
      }

      if (found) {
        for (uint32_t j = i; j < propCount; j++) {
          JS_FreeAtom(ctx, props[j].atom);
        }
        js_free(ctx, props);
        return propVal.release();
      }
      JS_FreeAtom(ctx, props[i].atom);
    }
    js_free(ctx, props);
  }
  return JS_UNDEFINED;
}
