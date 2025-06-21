#include "engines/quickjs/quickjs_engine_impl.h"
#include "engines/quickjs/quickjs_code_loader.h"
#include "quickjs.h"

QuickJsEngineImpl::QuickJsEngineImpl()
    : runtime_(JS_NewRuntime()), context_(JS_NewContext(runtime_)) {
  // Do not trigger GC when heap size is less than 16MB
  // default: rt->malloc_gc_threshold = 256 * 1024
  constexpr size_t SIXTEEN_MEGABYTES = 16L * 1024 * 1024;
  JS_SetGCThreshold(runtime_, SIXTEEN_MEGABYTES);
  JS_SetModuleLoaderFunc(runtime_, nullptr, js_module_loader, nullptr);

#ifdef WIN32
  /* use 0 to disable maximum stack size check */
  JS_SetMaxStackSize(runtime_, 0);
#endif

  exposeLogToJsConsole(context_);
}

QuickJsEngineImpl::~QuickJsEngineImpl() {
  JS_FreeContext(context_);
  JS_FreeRuntime(runtime_);
  registeredTypes_.clear();
}

int64_t QuickJsEngineImpl::getMemoryUsage() const {
  JSMemoryUsage qjsMemStats;
  JS_ComputeMemoryUsage(runtime_, &qjsMemStats);
  return qjsMemStats.memory_used_size;
}

size_t QuickJsEngineImpl::getArrayLength(const JSValue& array) const {
  auto lengthVal = JS_GetPropertyStr(context_, array, "length");
  if (JS_IsException(lengthVal)) {
    return 0;
  }
  return toInt(lengthVal);
}

void QuickJsEngineImpl::insertItemToArray(JSValue array, size_t index, const JSValue& value) const {
  JS_SetPropertyUint32(context_, array, index, value);
}

JSValue QuickJsEngineImpl::getArrayItem(const JSValue& array, size_t index) const {
  return JS_GetPropertyUint32(context_, array, index);
}

JSValue QuickJsEngineImpl::getObjectProperty(const JSValue& obj, const char* propertyName) const {
  return JS_GetPropertyStr(context_, obj, propertyName);
}

int QuickJsEngineImpl::setObjectProperty(JSValue obj,
                                         const char* propertyName,
                                         const JSValue& value) const {
  if (JS_IsUndefined(value) || JS_IsNull(value)) {
    // calling `JS_DeleteProperty` on globalThis would crash the program as well.
    LOG(ERROR) << "[qjs] Setting undefined or null to an object's property would crash the "
                  "program. Aborting.";
    return -1;
  }
  return JS_SetPropertyStr(context_, obj, propertyName, value);
}

int QuickJsEngineImpl::setObjectFunction(JSValue obj,
                                         const char* functionName,
                                         JSCFunction* cppFunction,
                                         int expectingArgc) const {
  return JS_SetPropertyStr(context_, obj, functionName,
                           JS_NewCFunction(context_, cppFunction, functionName, expectingArgc));
}

JSValue QuickJsEngineImpl::callFunction(const JSValue& func,
                                        const JSValue& thisArg,
                                        int argc,
                                        JSValue* argv) const {
  auto ret = JS_Call(context_, func, thisArg, argc, argv);
  if (JS_IsException(ret)) {
    logErrorStackTrace(ret, __FILE_NAME__, __LINE__);
  }
  return ret;
}

JSValue QuickJsEngineImpl::newClassInstance(const JSValue& clazz, int argc, JSValue* argv) const {
  JSValue constructor = getMethodOfClassOrInstance(clazz, JS_UNDEFINED, "constructor");
  auto instance = JS_CallConstructor(context_, constructor, argc, argv);
  JS_FreeValue(context_, constructor);
  return instance;
}

JSValue QuickJsEngineImpl::getJsClassHavingMethod(const JSValue& module,
                                                  const char* methodName) const {
  return QuickJSCodeLoader::getExportedClassHavingMethodNameInModule(context_, module, methodName);
}

JSValue QuickJsEngineImpl::getMethodOfClassOrInstance(JSValue jsClass,
                                                      JSValue instance,
                                                      const char* methodName) const {
  return QuickJSCodeLoader::getMethodByNameInClass(context_, jsClass, methodName);
}

void QuickJsEngineImpl::logErrorStackTrace(const JSValue& exception,
                                           const char* file,
                                           int line) const {
  QuickJSCodeLoader::logJsError(context_, "", file, line);
}

JSValue QuickJsEngineImpl::createInstanceOfModule(const char* moduleName,
                                                  std::vector<JSValue>& args,
                                                  const std::string& mainFuncName) const {
  return QuickJSCodeLoader::createInstanceOfEsmBundledModule(context_, moduleName, args,
                                                             mainFuncName);
}
JSValue QuickJsEngineImpl::loadJsFile(const char* fileName) const {
  return QuickJSCodeLoader::loadJsModuleToNamespace(context_, fileName);
}

JSValue QuickJsEngineImpl::eval(const char* code, const char* filename) const {
  return JS_Eval(context_, code, strlen(code), filename, JS_EVAL_TYPE_GLOBAL);
}

JSValue QuickJsEngineImpl::getGlobalObject() const {
  return JS_GetGlobalObject(context_);
}

JSValue QuickJsEngineImpl::throwError(JsErrorType errorType, const std::string& message) const {
  switch (errorType) {
    case JsErrorType::SYNTAX:
    case JsErrorType::EVAL:
      return JS_ThrowSyntaxError(context_, "%s", message.c_str());
    case JsErrorType::RANGE:
      return JS_ThrowRangeError(context_, "%s", message.c_str());
    case JsErrorType::REFERENCE:
      return JS_ThrowReferenceError(context_, "%s", message.c_str());
    case JsErrorType::TYPE:
      return JS_ThrowTypeError(context_, "%s", message.c_str());
    case JsErrorType::INTERNAL:
      return JS_ThrowInternalError(context_, "%s", message.c_str());
    case JsErrorType::GENERIC:
    case JsErrorType::UNKNOWN:
    default:
      return JS_ThrowPlainError(context_, "%s", message.c_str());
  }
}

void QuickJsEngineImpl::registerType(const char* typeName,
                                     JSClassID& classId,
                                     JSClassDef& classDef,
                                     JSCFunction* constructor,
                                     int constructorArgc,
                                     JSClassFinalizer* finalizer,
                                     const JSCFunctionListEntry* properties,
                                     int propertyCount,
                                     const JSCFunctionListEntry* getters,
                                     int getterCount,
                                     const JSCFunctionListEntry* functions,
                                     int functionCount) {
  // using WRAPPER = JsWrapper<T_RIME_TYPE>;

  if (registeredTypes_.count(typeName) > 0) {
    DLOG(INFO) << "type [" << typeName
               << "] has already been registered with classId = " << classId;
    return;
  }

  if (finalizer != nullptr) {
    classDef.finalizer = finalizer;
  }
  JS_NewClassID(runtime_, &classId);
  JS_NewClass(runtime_, classId, &classDef);

  registeredTypes_[typeName] = classId;

  JSValue proto = JS_NewObject(context_);
  if (constructor != nullptr) {
    JSValue jsConstructor =
        JS_NewCFunction2(context_, constructor, typeName, constructorArgc, JS_CFUNC_constructor, 0);
    JS_SetConstructor(context_, jsConstructor, proto);
    auto jsGlobal = JS_GetGlobalObject(context_);
    JS_SetPropertyStr(context_, jsGlobal, typeName, jsConstructor);
    JS_FreeValue(context_, jsGlobal);
  }

  if (properties != nullptr) {
    JS_SetPropertyFunctionList(context_, proto, properties, propertyCount);
  }
  if (getters != nullptr) {
    JS_SetPropertyFunctionList(context_, proto, getters, getterCount);
  }
  if (functions != nullptr) {
    JS_SetPropertyFunctionList(context_, proto, functions, functionCount);
  }

  JS_SetClassProto(context_, classId, proto);
  DLOG(INFO) << "registered type [" << typeName << "] with classId = " << classId;
}

std::string QuickJsEngineImpl::toStdString(const JSValue& value) const {
  const char* str = JS_ToCString(context_, value);
  if (str == nullptr) {
    return {};
  }
  std::string ret(str);
  JS_FreeCString(context_, str);
  return ret;
}

double QuickJsEngineImpl::toDouble(const JSValue& value) const {
  double ret = 0;
  JS_ToFloat64(context_, &ret, value);
  return ret;
}

size_t QuickJsEngineImpl::toInt(const JSValue& value) const {
  uint32_t ret = 0;
  JS_ToUint32(context_, &ret, value);
  return ret;
}

JSValue QuickJsEngineImpl::wrap(const char* typeName, void* ptr, const char* pointerType) const {
  if (ptr == nullptr) {
    return JS_NULL;
  }
  auto it = registeredTypes_.find(typeName);
  if (it == registeredTypes_.end()) {
    return JS_ThrowInternalError(context_, "Type %s is not registered", typeName);
  }

  JSValue jsobj = JS_NewObjectClass(context_, static_cast<int>(it->second));
  if (JS_IsUndefined(jsobj)) {
    LOG(ERROR) << "Failed to create a new object of type " << typeName
               << " with classId = " << it->second;
    return jsobj;
  }
  if (JS_IsException(jsobj)) {
    logErrorStackTrace(jsobj, __FILE_NAME__, __LINE__);
  }
  if (JS_SetOpaque(jsobj, ptr) < 0) {
    JS_FreeValue(context_, jsobj);
    const char* format = "Failed to set a %s pointer to a %s object with classId = %d";
    return JS_ThrowInternalError(context_, format, pointerType, typeName, it->second);
  }
  return jsobj;
}

void QuickJsEngineImpl::exposeLogToJsConsole(JSContext* ctx) {
  JSValue globalObj = JS_GetGlobalObject(ctx);
  JSValue console = JS_NewObject(ctx);
  JS_SetPropertyStr(ctx, console, "log", JS_NewCFunction(ctx, jsLog, "log", 1));
  JS_SetPropertyStr(ctx, console, "error", JS_NewCFunction(ctx, jsError, "error", 1));
  JS_SetPropertyStr(ctx, globalObj, "console", console);
  JS_FreeValue(ctx, globalObj);
}

static std::string logToStringStream(JSContext* ctx, int argc, JSValueConst* argv) {
  // FIXME: Unicode characters display incorrectly on Windows, showing "鉁?" instead of "✓".
  // While quickjs-ng's `js_print` function could help with console output,
  // I haven't found a way to integrate it with glog's `LOG(severity)` statements.
  // See related fix: https://github.com/quickjs-ng/quickjs/pull/449/files
  std::ostringstream oss;
  for (int i = 0; i < argc; i++) {
    const char* str = JS_ToCString(ctx, argv[i]);
    if (str != nullptr) {
      oss << (i != 0 ? " " : "") << str;
      JS_FreeCString(ctx, str);
    }
  }
  return oss.str();
}

JSValue QuickJsEngineImpl::jsLog(JSContext* ctx,
                                 JSValueConst thisVal,
                                 int argc,
                                 JSValueConst* argv) {
  google::LogMessage("$qjs$", 0, google::GLOG_INFO).stream() << logToStringStream(ctx, argc, argv);
  return JS_UNDEFINED;
}

JSValue QuickJsEngineImpl::jsError(JSContext* ctx,
                                   JSValueConst thisVal,
                                   int argc,
                                   JSValueConst* argv) {
  google::LogMessage("$qjs$", 0, google::GLOG_ERROR).stream() << logToStringStream(ctx, argc, argv);
  return JS_UNDEFINED;
}
