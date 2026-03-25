#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node_module_loader.h"
#include "node_module_loader_os.h"

#define LOG_AND_RETURN_ERROR(ctx, format, ...) \
  do { \
    logError(format, __VA_ARGS__); \
    return JS_ThrowReferenceError(ctx, format, __VA_ARGS__); \
  } while (0)

#define LOG_AND_THROW_ERROR(ctx, format, ...) \
  do { \
    logError(format, __VA_ARGS__); \
    JS_ThrowReferenceError(ctx, format, __VA_ARGS__); \
  } while (0)

enum { LOADER_PATH_MAX = 1024, MAX_BASE_FOLDERS = 5 };

#ifdef BUILD_FOR_QJS_EXE
static void log_v(FILE* stream, const char* format, va_list args) {
  vfprintf(stream, format, args);
  fputc('\n', stream);
}

void logInfo(const char* format, ...) {
  va_list args;
  va_start(args, format);
  log_v(stdout, format, args);
  va_end(args);
}

void logError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  log_v(stderr, format, args);
  va_end(args);
}
#else

extern void logInfoImpl(const char* message);
extern void logErrorImpl(const char* message);

static void log_to_impl(int is_error, const char* format, va_list args) {
  char buffer[LOADER_PATH_MAX];
  vsnprintf(buffer, sizeof(buffer), format, args);
  if (is_error) {
    logErrorImpl(buffer);
  } else {
    logInfoImpl(buffer);
  }
}

void logInfo(const char* format, ...) {
  va_list args;
  va_start(args, format);
  log_to_impl(0, format, args);
  va_end(args);
}

void logError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  log_to_impl(1, format, args);
  va_end(args);
}
#endif

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static char qjsBaseFolders[MAX_BASE_FOLDERS][LOADER_PATH_MAX] = {{0}};
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static int qjsBaseFoldersCount = 0;

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

__attribute__((constructor)) void initBaseFolder() {
  char path[LOADER_PATH_MAX] = {0};
  if (osGetExecutablePath(path, sizeof(path)) == 0) {
#if defined(_WIN32)
    char* lastSlash = strrchr(path, '\\');
#else
    char* lastSlash = strrchr(path, '/');
#endif
    if (lastSlash) *lastSlash = '\0';
    setQjsBaseFolder(path);
  }
}

/**
 * get the type of a given path (following symlinks)
 * @param path the path to check
 * @return FileType enum value
 */
static FileType getFileType(const char* path) {
  if (path == NULL) {
    return FILE_TYPE_ERROR;
  }

  StatStruct st;
  if (STAT_FUNC(path, &st) != 0) {
    if (errno == ENOENT) {
      return FILE_TYPE_NOT_EXIST;
    } else {
      return FILE_TYPE_ERROR; // other errors (e.g., permission denied)
    }
  }

  if (S_ISDIR(st.st_mode)) {
    return FILE_TYPE_DIR;
  } else if (S_ISREG(st.st_mode)) {
    return FILE_TYPE_REG;
  } else {
    return FILE_TYPE_OTHER; // other type (like device, pipe, etc.)
  }
}

static void joinPath(char* dest, size_t size, const char* path1, const char* path2) {
  if (!path1 || path1[0] == '\0') {
    strncpy(dest, path2, size - 1);
    dest[size - 1] = '\0';
  } else {
    size_t len1 = strlen(path1);
    const char* sep = (path1[len1 - 1] == '/' || path1[len1 - 1] == '\\') ? "" : "/";
    snprintf(dest, size, "%s%s%s", path1, sep, path2);
  }
}

static int hasJsExtension(const char* filename) {
  const char* extensions[] = {".js", ".mjs", ".cjs"};
  const int numExtensions = sizeof(extensions) / sizeof(extensions[0]);
  size_t len = strlen(filename);
  for (int i = 0; i < numExtensions; i++) {
    size_t extLen = strlen(extensions[i]);
    if (len > extLen && strcmp(filename + len - extLen, extensions[i]) == 0) {
      return 1;
    }
  }
  return 0;
}

static const char* findInBaseFolders(const char* subPath) {
  static char fullPath[LOADER_PATH_MAX];
  for (int i = 0; i < qjsBaseFoldersCount; i++) {
    joinPath(fullPath, sizeof(fullPath), qjsBaseFolders[i], subPath);
    if (getFileType(fullPath) == FILE_TYPE_REG) return fullPath;
  }
  return NULL;
}

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
    if (foundPath) return foundPath;
  }
  return NULL;
}

static const char* getActualFilePath(const char* path) {
  const char* possibleExtensions[] = {"", ".js", ".mjs", ".cjs"};
  const int numExtensions = sizeof(possibleExtensions) / sizeof(possibleExtensions[0]);

  static char fullPath[LOADER_PATH_MAX];
  for (int i = 0; i < numExtensions; i++) {
    snprintf(fullPath, sizeof(fullPath), "%s%s", path, possibleExtensions[i]);
    if (getFileType(fullPath) == FILE_TYPE_REG) {
      return fullPath;
    }
  }

  return NULL;
}

static char* parsePackageJsonForKey(const char* folder, const char* key) {
  char packageJsonPath[LOADER_PATH_MAX];
  joinPath(packageJsonPath, sizeof(packageJsonPath), folder, "package.json");

  FILE* fp = fopen(packageJsonPath, "r");
  if (!fp) return NULL;

  char line[LOADER_PATH_MAX];
  char* entryFileName = NULL;
  while (fgets(line, sizeof(line), fp)) {
    char* pos = strstr(line, key);
    if (!pos) continue;

    char* start = strchr(pos + strlen(key), '\"');
    if (!start) continue;
    start++;
    char* end = strchr(start, '\"');
    if (!end) continue;

    size_t len = end - start;
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

char* tryFindNodeModuleEntryFileName(const char* folder) {
  const char* keys[] = {"\"module\":", "\"main\":"};
  for (int i = 0; i < 2; i++) {
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

char* tryFindNodeModuleEntryPath(const char* moduleName) {
  char folder[LOADER_PATH_MAX];
  char nodeModulesSubPath[LOADER_PATH_MAX];
  joinPath(nodeModulesSubPath, sizeof(nodeModulesSubPath), "node_modules", moduleName);

  for (int j = 0; j < qjsBaseFoldersCount; j++) {
    joinPath(folder, sizeof(folder), qjsBaseFolders[j], nodeModulesSubPath);
    char* entryFileName = tryFindNodeModuleEntryFileName(folder);
    if (entryFileName) return entryFileName;
  }
  logError("Failed to find the entry file of the node module: %s", moduleName);
  return NULL;
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

  const size_t read = fread(content, 1, length, file);
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
  return osIsAbsolutePath(path) != 0;
}

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

JSValue loadJsModule(JSContext* ctx, const char* moduleName) {
  char* code = loadFile(moduleName);  // attempt to load the file directly first
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
      JSValue funcObj = loadJsModule(ctx, moduleName);
      if (JS_IsException(funcObj)) return NULL;
      JSModuleDef* m = (JSModuleDef*)JS_VALUE_GET_PTR(funcObj);
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

    JSValue funcObj = loadJsModule(ctx, subPath);
    if (JS_IsException(funcObj)) return NULL;
    JSModuleDef* m = (JSModuleDef*)JS_VALUE_GET_PTR(funcObj);
    JS_FreeValue(ctx, funcObj);
    return m;
  }

  logError("Failed to load js module: %s", moduleName);
  return NULL;
}
