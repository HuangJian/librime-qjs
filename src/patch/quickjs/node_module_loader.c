#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node_module_loader.h"

#define LOG_AND_RETURN_ERROR(ctx, format, ...) \
  logError(format, __VA_ARGS__); \
  return JS_ThrowReferenceError(ctx, format, __VA_ARGS__);

#define LOG_AND_THROW_ERROR(ctx, format, ...) \
  logError(format, __VA_ARGS__); \
  JS_ThrowReferenceError(ctx, format, __VA_ARGS__);

enum { LOADER_PATH_MAX = 1024 };

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static char qjsBaseFolder[LOADER_PATH_MAX] = {0};

void setQjsBaseFolder(const char* path) {
  strncpy(qjsBaseFolder, path, LOADER_PATH_MAX);
}

#ifdef _WIN32
#include <windows.h>
__attribute__((constructor)) void initBaseFolder() {
  char path[LOADER_PATH_MAX];
  GetModuleFileNameA(NULL, path, LOADER_PATH_MAX);
  char* last_slash = strrchr(path, '\\');
  if (last_slash) {
    *last_slash = '\0';
    setQjsBaseFolder(path);
  }
}
#endif

#ifdef __APPLE__
#include <unistd.h>
#include <limits.h>
#include <mach-o/dyld.h>  // For _NSGetExecutablePath on macOS

__attribute__((constructor)) void initBaseFolder() {
  char path[PATH_MAX];
  uint32_t size = PATH_MAX;
  // macOS specific path retrieval
  if (_NSGetExecutablePath(path, &size) == 0) {
    char* lastSlash = strrchr(path, '/');
    if (lastSlash) {
      *lastSlash = '\0';
      setQjsBaseFolder(path);
    }
  }
}
#endif

#ifdef __linux__
#include <unistd.h>
#include <limits.h>

__attribute__((constructor)) void initBaseFolder() {
  char path[PATH_MAX];
  uint32_t size = PATH_MAX;
  // Linux path retrieval
  ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
  if (count != -1) {
    path[count] = '\0';  // Ensure null termination
    char* lastSlash = strrchr(path, '/');
    if (lastSlash) {
      *lastSlash = '\0';
      setQjsBaseFolder(path);
    }
  }
}
#endif

#ifdef BUILD_FOR_QJS_EXE
void logInfo(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  printf("\n");
}

void logError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fwrite("\n", 1, 1, stderr);
}
#else

// Use extern "C" to properly interface with C++ glog library
#ifdef __cplusplus
#include <glog/logging.h>
#else
// For C code, declare external logging functions that will be implemented in C++
extern void logInfoImpl(const char* message);
extern void logErrorImpl(const char* message);
#endif

void logInfo(const char* format, ...) {
  va_list args;
  va_start(args, format);
  char buffer[LOADER_PATH_MAX];
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

#ifdef __cplusplus
  LOG(INFO) << buffer << "\n";
#else
  logInfoImpl(buffer);
#endif
}

void logError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  char buffer[LOADER_PATH_MAX];
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

#ifdef __cplusplus
  LOG(ERROR) << buffer << "\n";
#else
  logErrorImpl(buffer);
#endif
}

#endif

static bool isFileExists(const char* path) {
  FILE* file = fopen(path, "r");
  if (file) {
    fclose(file);
    return true;
  }
  return false;
}

static const char* getActualFileName(const char* moduleName) {
  static char fileNameAttempt[LOADER_PATH_MAX];
  char fullPath[LOADER_PATH_MAX];

  // has extension and file exists, return moduleName without modification
  const char* extensions[] = {".js", ".mjs", ".cjs"};
  const int numExtensions = sizeof(extensions) / sizeof(extensions[0]);
  for (int i = 0; i < numExtensions; i++) {
    unsigned long extLen = strlen(extensions[i]);
    if (strlen(moduleName) > extLen && strcmp(moduleName + strlen(moduleName) - extLen, extensions[i]) == 0) {
      snprintf(fullPath, sizeof(fullPath), "%s/%s", qjsBaseFolder, moduleName);
      return isFileExists(fullPath) ? moduleName : NULL;
    }
  }

  // file lookup order: ./dist/module.esm.js > ./dist/module.js > ./module.esm.js > ./module.js
  const char* filePatterns[] = {"dist/%s.esm.js", "dist/%s.js", "%s.esm.js", "%s.js"};
  const int numPatterns = sizeof(filePatterns) / sizeof(filePatterns[0]);

  for (int i = 0; i < numPatterns; i++) {
    snprintf(fileNameAttempt, sizeof(fileNameAttempt), filePatterns[i], moduleName);
    snprintf(fullPath, sizeof(fullPath), "%s/%s", qjsBaseFolder, fileNameAttempt);
    if (isFileExists(fullPath)) {
      return fileNameAttempt;
    }
  }
  return NULL;
}

static const char* getActualFilePath(const char* path) {
  const char* possibleExtensions[] = {"", ".js", ".mjs", ".cjs"};
  const int numExtensions = sizeof(possibleExtensions) / sizeof(possibleExtensions[0]);

  static char fullPath[LOADER_PATH_MAX];
  for (int i = 0; i < numExtensions; i++) {
    snprintf(fullPath, sizeof(fullPath), "%s%s", path, possibleExtensions[i]);
    if (isFileExists(fullPath)) {
      return fullPath;
    }
  }

  return NULL;
}

char* tryFindNodeModuleEntryFileName(const char* folder, const char* key) {
  char packageJsonPath[LOADER_PATH_MAX];
  snprintf(packageJsonPath, sizeof(packageJsonPath), "%s/package.json", folder);

  FILE* packageJson = fopen(packageJsonPath, "r");
  if (!packageJson) {
    return NULL;
  }

  char* entryFileName = NULL;
  char line[LOADER_PATH_MAX];

  while (fgets(line, sizeof(line), packageJson)) {
    char* pos = strstr(line, key);
    if (!pos) {
      continue;
    }

    char* leftQuote = strchr(pos + strlen(key), '\"');
    if (!leftQuote) {
      continue;
    }

    char* rightQuote = strchr(leftQuote + 1, '\"');
    if (!rightQuote) {
      continue;
    }

    size_t len = rightQuote - (leftQuote + 1);
    entryFileName = (char*)malloc(len + 1);
    strncpy(entryFileName, leftQuote + 1, len);
    entryFileName[len] = '\0';

    logInfo("Found entry file: [%s] from package.json line: %s", entryFileName, line);
    break;
  }

  fclose(packageJson);

  char entryFilePath[LOADER_PATH_MAX];
  snprintf(entryFilePath, sizeof(entryFilePath), "%s/%s", folder, entryFileName);
  const char* actualPath = getActualFilePath(entryFilePath);
  if (actualPath) {
    return entryFileName;
  }

  return NULL;
}

char* tryFindNodeModuleEntryPath(const char* baseFolder,
                                 const char* moduleName) {
  char folder[LOADER_PATH_MAX];
  snprintf(folder, sizeof(folder), "%s/node_modules/%s", baseFolder, moduleName);

  char* entryFileName = tryFindNodeModuleEntryFileName(folder, "\"module\":");
  if (!entryFileName) {
    entryFileName = tryFindNodeModuleEntryFileName(folder, "\"main\":");
  }

  if (!entryFileName) {
    logError("Failed to find the entry file of the node module: %s", moduleName);
    return NULL;
  }

  return entryFileName;
}

char* loadFile(const char* absolutePath) {
  const char* actualPath  = getActualFilePath(absolutePath);
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
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (length <= 0) {
    logError("Invalid file length: %ld for file: %s", length, absolutePath);
    fclose(file);
    return NULL;
  }

  char* content = (char*)malloc(length + 1);
  if (!content) {
    logError("Failed to allocate memory for file: %s", absolutePath);
    fclose(file);
    return NULL;
  }

  size_t read = fread(content, 1, length, file);
  fclose(file);

  if (read != (size_t)length) {
    logError("Failed to read file: %s, expected %ld bytes but got %zu", absolutePath, length, read);
    free(content);
    return NULL;
  }

  content[length] = '\0';
  return content;
}
bool isAbsolutePath(const char* path) {
  size_t len = strlen(path);
  return (len > 0 && path[0] == '/') ||  // Unix-style absolute path
         (len > 1 && isalpha(path[0]) && path[1] == ':') || // Windows drive letter (e.g., C:)
         (len > 1 && path[0] == '\\' && path[1] == '\\');  // Windows UNC path
}

char* readJsCode(JSContext* ctx, const char* moduleName) {
  if (strlen(qjsBaseFolder) == 0) {
    LOG_AND_THROW_ERROR(ctx, "basePath is empty in loading js file: %s", moduleName);
    return NULL;
  }

  const char* fileName = getActualFileName(moduleName);
  if (!fileName) {
    LOG_AND_THROW_ERROR(ctx, "File not found: %s", moduleName);
    return NULL;
  }
  char fullPath[LOADER_PATH_MAX];
  if (isAbsolutePath(fileName)) {
    snprintf(fullPath, sizeof(fullPath), "%s", fileName);
  } else {
    snprintf(fullPath, sizeof(fullPath), "%s/%s", qjsBaseFolder, fileName);
  }

  return loadFile(fullPath);
}

JSValue loadJsModule(JSContext* ctx, const char* moduleName) {
  char* code = readJsCode(ctx, moduleName);
  if (!code) {
    LOG_AND_RETURN_ERROR(ctx, "Could not open %s", moduleName);
  }

  size_t codeLen = strlen(code);
  if (codeLen == 0) {
    free(code);
    LOG_AND_RETURN_ERROR(ctx, "Empty module content: %s", moduleName);
  }

  int flags = JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY;
  JSValue funcObj = JS_Eval(ctx, code, codeLen, moduleName, flags);
  free(code);

  if (JS_IsException(funcObj)) {
    JSValue exception = JS_GetException(ctx);
    JSValue message = JS_GetPropertyStr(ctx, exception, "message");
    const char* messageStr = JS_ToCString(ctx, message);
    logError("Module evaluation failed: %s", messageStr);

    JS_FreeCString(ctx, messageStr);
    JS_FreeValue(ctx, message);
    JS_FreeValue(ctx, exception);
  }
  return funcObj;
}

// NOLINTNEXTLINE(readability-identifier-naming)
JSModuleDef* js_module_loader(JSContext* ctx,
                              const char* moduleName,
                              void* opaque) {
  char fullPath[LOADER_PATH_MAX];
  if (isAbsolutePath(moduleName)) {
    snprintf(fullPath, sizeof(fullPath), "%s", moduleName);
  } else {
    snprintf(fullPath, sizeof(fullPath), "%s/%s", qjsBaseFolder, moduleName);
  }

  const char* actualPath = getActualFilePath(fullPath);
  if (actualPath) { // file exists, it's a js file outside node_modules
    JSValue funcObj = loadJsModule(ctx, moduleName);
    return (JSModuleDef*)JS_VALUE_GET_PTR(funcObj);
  }

  // try to load the file from node_modules
  char* nodeModuleEntryFile = tryFindNodeModuleEntryPath(qjsBaseFolder, moduleName);
  if (!nodeModuleEntryFile) {
    logError("Failed to load the js module: %s", moduleName);
    return NULL;
  }

  char modulePath[LOADER_PATH_MAX];
  snprintf(modulePath, sizeof(modulePath), "node_modules/%s/%s", moduleName, nodeModuleEntryFile);
  free(nodeModuleEntryFile);

  JSValue funcObj = loadJsModule(ctx, modulePath);
  return (JSModuleDef*)JS_VALUE_GET_PTR(funcObj);
}
