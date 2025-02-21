#include "qjs_trie.h"

namespace rime {

static JSValue loadFile(JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
  const char* absolutePath = JS_ToCString(ctx, argv[0]);
  auto obj = std::make_shared<Trie>();
  return QjsTrie::Wrap(ctx, obj);
}

DEFINE_JS_CLASS_WITH_SHARED_POINTER(
  Trie,
  DEFINE_CONSTRUCTOR(Trie, loadFile, 1),
  NO_PROPERTY_TO_REGISTER,
  DEFINE_FUNCTIONS(
    JS_CFUNC_DEF("loadTextFile", 2, loadTextFile),
    JS_CFUNC_DEF("loadBinaryFile", 1, loadBinaryFile),
    JS_CFUNC_DEF("saveToBinaryFile", 1, saveToBinaryFile),
    JS_CFUNC_DEF("find", 1, find),
    JS_CFUNC_DEF("prefixSearch", 1, prefixSearch)
  )
)

DEF_FUNC_WITH_ARGC(Trie, loadTextFile, 2,
  JSStringRAII str(JS_ToCString(ctx, argv[0]));
  std::string absolutePath(str);
  int32_t size;
  JS_ToInt32(ctx, &size, argv[1]);

  try {
    obj->loadTextFile(absolutePath, size);
  } catch (const std::exception& e) {
    LOG(ERROR) << "loadTextFile of " << absolutePath << " failed: " << e.what();
    return JS_ThrowPlainError(ctx, "%s", e.what());
  }

  return JS_UNDEFINED;
)

DEF_FUNC_WITH_ARGC(Trie, loadBinaryFile, 1,
  JSStringRAII str(JS_ToCString(ctx, argv[0]));
  std::string absolutePath(str);
  try {
    obj->loadBinaryFileMmap(absolutePath);
  } catch (const std::exception& e) {
    LOG(ERROR) << "loadBinaryFileMmap of " << absolutePath << " failed: " << e.what();
    return JS_ThrowPlainError(ctx, "%s", e.what());
  }
  return JS_UNDEFINED;
)

DEF_FUNC_WITH_ARGC(Trie, saveToBinaryFile, 1,
  JSStringRAII str(JS_ToCString(ctx, argv[0]));
  std::string absolutePath(str);
  try {
    obj->saveToBinaryFile(absolutePath);
  } catch (const std::exception& e) {
    LOG(ERROR) << "saveToBinaryFile of " << absolutePath << " failed: " << e.what();
    return JS_ThrowPlainError(ctx, "%s", e.what());
  }
  return JS_UNDEFINED;
)

DEF_FUNC_WITH_ARGC(Trie, find, 1,
  JSStringRAII key(JS_ToCString(ctx, argv[0]));
  auto result = obj->find(std::string(key));
  return result.has_value() ? JS_NewString(ctx, result.value().c_str()) : JS_NULL;
)

DEF_FUNC_WITH_ARGC(Trie, prefixSearch, 1,
  JSStringRAII prefix(JS_ToCString(ctx, argv[0]));
  auto matches = obj->prefix_search(std::string(prefix));

  JSValue jsArray = JS_NewArray(ctx);
  for (size_t i = 0; i < matches.size(); ++i) {
    JSValue jsObject = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, jsObject, "text", JS_NewString(ctx, matches[i].first.c_str()));
    JS_SetPropertyStr(ctx, jsObject, "info", JS_NewString(ctx, matches[i].second.c_str()));
    JS_SetPropertyUint32(ctx, jsArray, i, jsObject);
  }
  return jsArray;
)

} // namespace rime
