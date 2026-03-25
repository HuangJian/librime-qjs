#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node_module_loader.h"

#define LOG_AND_RETURN_ERROR(ctx, format, ...)     \
    logError(format, ##__VA_ARGS__);               \
    return JS_ThrowReferenceError(ctx, format, ##__VA_ARGS__);

#define LOG_AND_THROW_ERROR(ctx, format, ...)      \
    logError(format, ##__VA_ARGS__);               \
    JS_ThrowReferenceError(ctx, format, ##__VA_ARGS__);

enum { LOADER_PATH_MAX = 1024, MAX_BASE_FOLDERS = 5 };

#ifdef BUILD_FOR_QJS_EXE
/**
 * @brief Logs an informational message to stdout.
 *
 * @param stream Output stream.
 * @param format Printf-style format string.
 * @param args Variable arguments for the format string.
 */
static void logV(FILE* stream, const char* format, va_list args) {
  vfprintf(stream, format, args);
  fputc('\n', stream);
}

/**
 * @brief Logs an informational message to stdout.
 *
 * @param format Printf-style format string.
 * @param ... Variable arguments for the format string.
 */
void logInfo(const char* format, ...) {
  va_list args;
  va_start(args, format);
  logV(stdout, format, args);
  va_end(args);
}

/**
 * @brief Logs an error message to stderr.
 *
 * @param format Printf-style format string.
 * @param ... Variable arguments for the format string.
 */
void logError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  logV(stderr, format, args);
  va_end(args);
}
#else

extern void logInfoImpl(const char* message);
extern void logErrorImpl(const char* message);

/**
 * @brief Internal helper to log messages using the implementation provided by the host.
 *
 * @param isError Non-zero if this is an error message.
 * @param format Printf-style format string.
 * @param args Variable arguments for the format string.
 */
static void logToImpl(const int isError, const char* format, va_list args) {
  char buffer[LOADER_PATH_MAX];
  vsnprintf(buffer, sizeof(buffer), format, args);
  if (isError) {
    logErrorImpl(buffer);
  } else {
    logInfoImpl(buffer);
  }
}

/**
 * @brief Logs an informational message.
 *
 * @param format Printf-style format string.
 * @param ... Variable arguments.
 */
void logInfo(const char* format, ...) {
  va_list args;
  va_start(args, format);
  logToImpl(0, format, args);
  va_end(args);
}

/**
 * @brief Logs an error message.
 *
 * @param format Printf-style format string.
 * @param ... Variable arguments.
 */
void logError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  logToImpl(1, format, args);
  va_end(args);
}
#endif

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static char qjsBaseFolders[MAX_BASE_FOLDERS][LOADER_PATH_MAX] = {{0}};
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static int qjsBaseFoldersCount = 0;

/**
 * @brief Registers a base folder path for the module loader to search in.
 *
 * @param path The absolute or relative path to the folder.
 */
void setQjsBaseFolder(const char* path) {
  if (!path || strlen(path) == 0) {
    return;
  }

  // Check for duplicates
  for (int i = 0; i < qjsBaseFoldersCount; i++) {
    if (strcmp(qjsBaseFolders[i], path) == 0) {
      return;
    }
  }

  if (qjsBaseFoldersCount >= MAX_BASE_FOLDERS) {
    logError("Maximum number of base folders (%d) reached", MAX_BASE_FOLDERS);
    return;
  }

  strncpy(qjsBaseFolders[qjsBaseFoldersCount], path, LOADER_PATH_MAX - 1);
  qjsBaseFolders[qjsBaseFoldersCount][LOADER_PATH_MAX - 1] = '\0';
  qjsBaseFoldersCount++;
}

/**
 * @brief Automatically initializes the base search folder based on the executable's location.
 * Called as a constructor during program initialization.
 */
__attribute__((constructor)) void initBaseFolder() {
  char path[LOADER_PATH_MAX] = {0};
  if (getExecutablePath(path, sizeof(path)) == 0) {
#ifdef _WIN32
    char* lastSlash = strrchr(path, '\\');
#else
    char* lastSlash = strrchr(path, '/');
#endif
    if (lastSlash) { *lastSlash = '\0';
}
    setQjsBaseFolder(path);
  }
}

/**
 * @brief Retrieves the type of a filesystem entry.
 *
 * @param path The path to check.
 * @return FileType enum value (Reg, Dir, NotExist, etc.).
 */
static FileType getFileType(const char* path) {
  if (path == NULL) {
    return FileType_Error;
  }

  StatStruct st;
  if (STAT_FUNC(path, &st) != 0) {
    if (errno == ENOENT) {
      return FileType_NotExist;
    }
    return FileType_Error;  // other errors (e.g., permission denied)
  }

  if (S_ISDIR(st.st_mode)) {
    return FileType_Dir;
  }
  if (S_ISREG(st.st_mode)) {
    return FileType_Reg;
  }
  return FileType_Other;  // other type (like device, pipe, etc.)
}

/**
 * @brief Safely joins two path components into a single path string.
 *
 * @param dest Buffer to store the joined path.
 * @param size Size of the destination buffer.
 * @param path1 The first part of the path.
 * @param path2 The second part of the path.
 */
static void joinPath(char* dest, const size_t size, const char* path1, const char* path2) {
  if (!path1 || path1[0] == '\0') {
    strncpy(dest, path2, size - 1);
    dest[size - 1] = '\0';
  } else {
    const size_t len1 = strlen(path1);
    const char* sep = path1[len1 - 1] == '/' || path1[len1 - 1] == '\\' ? "" : "/";
    snprintf(dest, size, "%s%s%s", path1, sep, path2);
  }
}

/**
 * @brief Checks if a filename has a recognized JavaScript module extension.
 *
 * @param filename The filename to check.
 * @return 1 if it has a JS extension, 0 otherwise.
 */
static int hasJsExtension(const char* filename) {
  const char* extensions[] = {".js", ".mjs", ".cjs"};
  const int numExtensions = sizeof(extensions) / sizeof(extensions[0]);
  const size_t len = strlen(filename);
  for (int i = 0; i < numExtensions; i++) {
    const size_t extLen = strlen(extensions[i]);
    if (len > extLen && strcmp(filename + len - extLen, extensions[i]) == 0) {
      return 1;
    }
  }
  return 0;
}

/**
 * @brief Searches for a subpath in all registered base folders.
 *
 * @param subPath The relative subpath to search for.
 * @return Pointer to a static buffer containing the full path if found, or NULL.
 */
static const char* findInBaseFolders(const char* subPath) {
  static char fullPath[LOADER_PATH_MAX];
  for (int i = 0; i < qjsBaseFoldersCount; i++) {
    joinPath(fullPath, sizeof(fullPath), qjsBaseFolders[i], subPath);
    if (getFileType(fullPath) == FileType_Reg) { return fullPath;
}
  }
  return NULL;
}

/**
 * @brief Resolves the full path of a module by searching for various patterns.
 * Patterns include: extension match, dist/ files, and plain module names.
 *
 * @param moduleName The name of the module to resolve.
 * @return Pointer to a static buffer containing the full path if resolved, or NULL.
 */
static const char* getModuleFullPath(const char* moduleName) {
  if (hasJsExtension(moduleName)) {
    return findInBaseFolders(moduleName);
  }

  const char* filePatterns[] = {"dist/%s.esm.js", "dist/%s.js", "%s.esm.js", "%s.js"};
  const int numPatterns = sizeof(filePatterns) / sizeof(filePatterns[0]);

  for (int i = 0; i < numPatterns; i++) {
    char fileNameAttempt[LOADER_PATH_MAX];
    snprintf(fileNameAttempt, sizeof(fileNameAttempt), filePatterns[i], moduleName);
    const char* foundPath = findInBaseFolders(fileNameAttempt);
    if (foundPath) { return foundPath;
}
  }
  return NULL;
}

/**
 * @brief Tries to find the actual file path by appending recognized extensions.
 *
 * @param path The base path to check.
 * @return Pointer to a static buffer containing the actual file path if found, or NULL.
 */
static const char* getActualFilePath(const char* path) {
  const char* possibleExtensions[] = {"", ".js", ".mjs", ".cjs"};
  const int numExtensions = sizeof(possibleExtensions) / sizeof(possibleExtensions[0]);

  static char fullPath[LOADER_PATH_MAX];
  for (int i = 0; i < numExtensions; i++) {
    snprintf(fullPath, sizeof(fullPath), "%s%s", path, possibleExtensions[i]);
    if (getFileType(fullPath) == FileType_Reg) {
      return fullPath;
    }
  }

  return NULL;
}

/**
 * @brief Parses package.json in a folder to find the entry point for a given key.
 *
 * @param folder The folder containing package.json.
 * @param key The key to look for (e.g., "module", "main").
 * @return Malloc'd string with the entry filename if found, or NULL.
 */
static char* parsePackageJsonForKey(const char* folder, const char* key) {
  char packageJsonPath[LOADER_PATH_MAX];
  joinPath(packageJsonPath, sizeof(packageJsonPath), folder, "package.json");

  FILE* fp = fopen(packageJsonPath, "r");
  if (!fp) { return NULL;
}

  char line[LOADER_PATH_MAX];
  char* entryFileName = NULL;
  while (fgets(line, sizeof(line), fp)) {
    char* pos = strstr(line, key);
    if (!pos) { continue;
}

    char* start = strchr(pos + strlen(key), '\"');
    if (!start) { continue;
}
    start++;
    char* end = strchr(start, '\"');
    if (!end) { continue;
}

    const size_t len = end - start;
    entryFileName = (char*)malloc(len + 1);
    if (entryFileName) {
      memcpy(entryFileName, start, len);
      entryFileName[len] = '\0';
      logInfo("Found entry file [%s] from package.json key [%s]", entryFileName, key);
      break;
    }
  }
  fclose(fp);
  return entryFileName;
}

/**
 * @brief Searches for a node module's entry point in a specific folder.
 * Checks "module" then "main" fields in package.json.
 *
 * @param folder The node module folder.
 * @return Malloc'd string with the entry filename if found, or NULL.
 */
char* tryFindNodeModuleEntryFileName(const char* folder) {
  const char* keys[] = {"\"module\":", "\"main\":"};
  for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
    char* entry = parsePackageJsonForKey(folder, keys[i]);
    if (entry) {
      char entryFilePath[LOADER_PATH_MAX];
      joinPath(entryFilePath, sizeof(entryFilePath), folder, entry);
      if (getActualFilePath(entryFilePath)) {
        return entry;
      }
      free(entry);
    }
  }
  return NULL;
}

/**
 * @brief Resolves a node module's entry file by searching in node_modules folders.
 *
 * @param moduleName The name of the node module.
 * @return Malloc'd string with the entry filename if found, or NULL.
 */
char* tryFindNodeModuleEntryPath(const char* moduleName) {
  char folder[LOADER_PATH_MAX];
  char nodeModulesSubPath[LOADER_PATH_MAX];
  joinPath(nodeModulesSubPath, sizeof(nodeModulesSubPath), "node_modules", moduleName);

  for (int j = 0; j < qjsBaseFoldersCount; j++) {
    joinPath(folder, sizeof(folder), qjsBaseFolders[j], nodeModulesSubPath);
    char* entryFileName = tryFindNodeModuleEntryFileName(folder);
    if (entryFileName) { return entryFileName;
}
  }
  logError("Failed to find the entry file of the node module: %s", moduleName);
  return NULL;
}

/**
 * @brief Reads the entire content of a file into a null-terminated string.
 *
 * @param absolutePath The path to the file.
 * @return Malloc'd string with file content, or NULL on error.
 */
char* loadFile(const char* absolutePath) {
  const char* actualPath = getActualFilePath(absolutePath);
  if (!actualPath) {
    logError("Failed to open file at: %s", absolutePath);
    return NULL;
  }

  FILE* file = fopen(actualPath, "rb");
  if (!file) {
    logError("Failed to open file at: %s", absolutePath);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  const long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (length <= 0) {
    logError("Invalid file length: %ld for file: %s", length, absolutePath);
    fclose(file);
    return NULL;
  }

  char* content = malloc(length + 1);
  if (!content) {
    logError("Failed to allocate memory for file: %s", absolutePath);
    fclose(file);
    return NULL;
  }

  const size_t readCount = fread(content, 1, (size_t)length, file);
  fclose(file);

  if (readCount != (size_t)length) {
    logError("Failed to read file: %s, expected %ld bytes but got %zu", absolutePath, length, readCount);
    free(content);
    return NULL;
  }

  content[length] = '\0';
  return content;
}

/**
 * @brief Locates and reads the source code of a JavaScript module.
 *
 * @param ctx QuickJS context.
 * @param moduleName Name of the module.
 * @return Malloc'd string with JS source code, or NULL.
 */
char* readJsCode(JSContext* ctx, const char* moduleName) {
  if (qjsBaseFoldersCount == 0) {
    LOG_AND_THROW_ERROR(ctx, "basePath is empty in loading js file: %s", moduleName);
    return NULL;
  }

  const char* fullPath = getModuleFullPath(moduleName);
  if (!fullPath) {
    LOG_AND_THROW_ERROR(ctx, "File not found: %s", moduleName);
    return NULL;
  }

  return loadFile(fullPath);
}

/**
 * @brief Compiles a JavaScript module and returns its function object.
 *
 * @param ctx QuickJS context.
 * @param moduleName Name of the module.
 * @return JSValue representing the compiled module, or JS_EXCEPTION.
 */
JSValue loadJsModule(JSContext* ctx, const char* moduleName) {
  char* code = loadFile(moduleName); // attempt to load the file directly first
  if (!code) {
    code = readJsCode(ctx, moduleName);
  }
  if (!code) {
    LOG_AND_RETURN_ERROR(ctx, "Could not open %s", moduleName);
  }

  const size_t codeLen = strlen(code);
  if (codeLen == 0) {
    free(code);
    LOG_AND_RETURN_ERROR(ctx, "Empty module content: %s", moduleName);
  }

  const int flags = JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY;
  const JSValue funcObj = JS_Eval(ctx, code, codeLen, moduleName, flags);
  free(code);

  if (JS_IsException(funcObj)) {
    const JSValue exception = JS_GetException(ctx);
    const JSValue message = JS_GetPropertyStr(ctx, exception, "message");
    const char* messageStr = JS_ToCString(ctx, message);
    logError("Module evaluation failed: %s", messageStr);

    JS_FreeCString(ctx, messageStr);
    JS_FreeValue(ctx, message);
    JS_FreeValue(ctx, exception);
  }
  return funcObj;
}

/**
 * @brief Standard QuickJS module loader callback implementation.
 * Resolves module names to files and compiles them into QuickJS modules.
 *
 * @param ctx QuickJS context.
 * @param moduleName Name of the module to load.
 * @param opaque User-provided opaque data.
 * @return JSModuleDef pointer on success, NULL on error.
 */
// NOLINTNEXTLINE(readability-identifier-naming)
JSModuleDef* js_module_loader(JSContext* ctx, const char* moduleName, void* opaque) {
  // 1. Try to find the file in any of the registered base folders (non-node_modules)
  for (int i = 0; i < qjsBaseFoldersCount; i++) {
    char fullPath[LOADER_PATH_MAX];
    if (isAbsolutePath(moduleName) || getActualFilePath(moduleName)) {
      strncpy(fullPath, moduleName, sizeof(fullPath) - 1);
      fullPath[sizeof(fullPath) - 1] = '\0';
    } else {
      joinPath(fullPath, sizeof(fullPath), qjsBaseFolders[i], moduleName);
    }

    if (getActualFilePath(fullPath)) {
      const JSValue funcObj = loadJsModule(ctx, moduleName);
      if (JS_IsException(funcObj)) { return NULL;
}
      JSModuleDef* m = JS_VALUE_GET_PTR(funcObj);
      JS_FreeValue(ctx, funcObj);
      return m;
    }
  }

  // 2. Try to load from node_modules
  char* entryFile = tryFindNodeModuleEntryPath(moduleName);
  if (entryFile) {
    char subPath[LOADER_PATH_MAX];
    snprintf(subPath, sizeof(subPath), "node_modules/%s/%s", moduleName, entryFile);
    free(entryFile);

    const JSValue funcObj = loadJsModule(ctx, subPath);
    if (JS_IsException(funcObj)) { return NULL;
}
    JSModuleDef* m = JS_VALUE_GET_PTR(funcObj);
    JS_FreeValue(ctx, funcObj);
    return m;
  }

  logError("Failed to load js module: %s", moduleName);
  return NULL;
}
