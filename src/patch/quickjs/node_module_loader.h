#ifndef NODE_MODULE_LOADER_H
#define NODE_MODULE_LOADER_H

#include "quickjs.h"

#ifdef __cplusplus
extern "C" {
#endif

void setQjsBaseFolder(const char* path);

// NOLINTNEXTLINE(readability-identifier-naming)
JSModuleDef* js_module_loader(JSContext* ctx, const char* moduleName, void* opaque);

JSValue loadJsModule(JSContext* ctx, const char* moduleName);

char* readJsCode(JSContext* ctx, const char* moduleName);

char* loadFile(const char* absolutePath);

#ifdef __cplusplus
}
#endif
#endif  // NODE_MODULE_LOADER_H