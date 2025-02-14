#ifndef RIME_QJS_TYPE_REGISTRY_H_
#define RIME_QJS_TYPE_REGISTRY_H_

#include <quickjs.h>
#include <string>

namespace rime {

// Base class for type registration
class QjsTypeRegistry {
public:
  virtual ~QjsTypeRegistry() = default;

  // Register the type with QuickJS runtime
  virtual void Register(JSContext* ctx) = 0;

  // Get the class name for this type
  virtual const char* GetClassName() const = 0;

protected:
  // Helper to register class methods
  void RegisterClassMethods(JSContext* ctx, JSValue proto, const JSCFunctionListEntry* methods, int count);

  // Helper to register class properties
  void RegisterClassProperties(JSContext* ctx, JSValue proto, const JSCFunctionListEntry* properties, int count);

  static JSValue js_new_string_from_std(JSContext* ctx, const std::string& str) {
    return JS_NewString(ctx, str.c_str());
  }
};

} // namespace rime

#endif // RIME_QJS_TYPE_REGISTRY_H_
