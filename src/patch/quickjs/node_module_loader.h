#ifndef NODE_MODULE_LOADER_H
#define NODE_MODULE_LOADER_H

#include "quickjs.h"

#ifdef _WIN32
#include "node_module_loader_win.h"
#elif defined(__APPLE__)
#include "node_module_loader_mac.h"
#elif defined(__linux__)
#include "node_module_loader_linux.h"
#endif

// If the system doesn't provide S_ISDIR / S_ISREG macros, define them manually.
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif

/**
 * @brief Represents the type of a filesystem entry.
 */
typedef enum {
  FileType_Error = -1,   /**< An error occurred during check. */
  FileType_NotExist = 0, /**< The path does not exist. */
  FileType_Reg = 1,      /**< The path is a regular file. */
  FileType_Dir = 2,      /**< The path is a directory. */
  FileType_Other = 3     /**< The path is of another type. */
} FileType;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Registers a base folder path for the module loader.
 *
 * @param path Pointer to the path string.
 */
void setQjsBaseFolder(const char* path);

/**
 * @brief QuickJS module loader callback.
 *
 * @param ctx QuickJS context.
 * @param moduleName Name of the module to load.
 * @param opaque User-provided opaque pointer.
 * @return JSModuleDef* on success, NULL on error.
 */
// NOLINTNEXTLINE(readability-identifier-naming)
JSModuleDef* js_module_loader(JSContext* ctx, const char* moduleName, void* opaque);

/**
 * @brief Compiles a JS module.
 *
 * @param ctx QuickJS context.
 * @param moduleName Name of the module.
 * @return JSValue Function object of the module.
 */
JSValue loadJsModule(JSContext* ctx, const char* moduleName);

/**
 * @brief Reads JS source code.
 *
 * @param ctx QuickJS context.
 * @param moduleName Name of the module.
 * @return char* Malloc'd source code string.
 */
char* readJsCode(JSContext* ctx, const char* moduleName);

/**
 * @brief Reads a file into a string.
 *
 * @param absolutePath Full path to the file.
 * @return char* Malloc'd content string.
 */
char* loadFile(const char* absolutePath);

#ifdef __cplusplus
}
#endif
#endif  // NODE_MODULE_LOADER_H
