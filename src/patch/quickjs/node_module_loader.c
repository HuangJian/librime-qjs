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

enum { LOADER_PATH_MAX = 1024, MAX_BASE_FOLDERS = 5 };

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
      // logInfo("Base folder already registered: %s", path);
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

#include <errno.h>
#ifdef _WIN32
    #include <sys/stat.h>
    typedef struct _stat StatStruct;
    #define STAT_FUNC _stat
#else
    #include <sys/stat.h>
    #include <unistd.h>
    typedef struct stat StatStruct;
    #define STAT_FUNC stat
#endif

// If the system doesn't provide S_ISDIR / S_ISREG macros, define them manually.
#ifndef S_ISDIR
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#endif

typedef enum {
    FILE_TYPE_ERROR = -1,    // error occurred while checking the file type (e.g., permission denied)
    FILE_TYPE_NOT_EXIST = 0, // path is not exist
    FILE_TYPE_REG = 1,       // regular file
    FILE_TYPE_DIR = 2,       // directory
    FILE_TYPE_OTHER = 3      // other type (like device, pipe, etc.)
} FileType;

/**
 * get the type of a given path (following symlinks)
 * @param path the path to check
 * @return FileType enum value
 */
static FileType getFileType(const char *path) {
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

    // get the file type
    if (S_ISDIR(st.st_mode)) {
        return FILE_TYPE_DIR;
    } else if (S_ISREG(st.st_mode)) {
        return FILE_TYPE_REG;
    } else {
        return FILE_TYPE_OTHER; // other type (like device, pipe, etc.)
    }
}

static const char* getModuleFullPath(const char* moduleName) {
  static char fullPath[LOADER_PATH_MAX];
  char fileNameAttempt[LOADER_PATH_MAX];

  // has extension and file exists, return moduleName without modification
  const char* extensions[] = {".js", ".mjs", ".cjs"};
  const int numExtensions = sizeof(extensions) / sizeof(extensions[0]);
  for (int i = 0; i < numExtensions; i++) {
    unsigned long extLen = strlen(extensions[i]);
    if (strlen(moduleName) > extLen && strcmp(moduleName + strlen(moduleName) - extLen, extensions[i]) == 0) {
      for (int j = 0; j < qjsBaseFoldersCount; j++) {
        snprintf(fullPath, sizeof(fullPath), "%s/%s", qjsBaseFolders[j], moduleName);
        if (getFileType(fullPath) == FILE_TYPE_REG) {
          return fullPath;
        }
      }
      return NULL;
    }
  }

  // file lookup order: ./dist/module.esm.js > ./dist/module.js > ./module.esm.js > ./module.js
  const char* filePatterns[] = {"dist/%s.esm.js", "dist/%s.js", "%s.esm.js", "%s.js"};
  const int numPatterns = sizeof(filePatterns) / sizeof(filePatterns[0]);

  for (int i = 0; i < numPatterns; i++) {
    snprintf(fileNameAttempt, sizeof(fileNameAttempt), filePatterns[i], moduleName);
    for (int j = 0; j < qjsBaseFoldersCount; j++) {
      snprintf(fullPath, sizeof(fullPath), "%s/%s", qjsBaseFolders[j], fileNameAttempt);
      if (getFileType(fullPath) == FILE_TYPE_REG) {
        return fullPath;
      }
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
    if (getFileType(fullPath) == FILE_TYPE_REG) {
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

char* tryFindNodeModuleEntryPath(const char* moduleName) {
  char folder[LOADER_PATH_MAX];

  for (int j = 0; j < qjsBaseFoldersCount; j++) {
    snprintf(folder, sizeof(folder), "%s/node_modules/%s", qjsBaseFolders[j], moduleName);

    char* entryFileName = tryFindNodeModuleEntryFileName(folder, "\"module\":");
    if (!entryFileName) {
      entryFileName = tryFindNodeModuleEntryFileName(folder, "\"main\":");
    }

    if (entryFileName) {
      return entryFileName;
    }
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
  const size_t len = strlen(path);
  return (len > 0 && path[0] == '/') ||  // Unix-style absolute path
         (len > 1 && isalpha(path[0]) && path[1] == ':') || // Windows drive letter (e.g., C:)
         (len > 1 && path[0] == '\\' && path[1] == '\\');  // Windows UNC path
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
JSModuleDef* js_module_loader(JSContext* ctx,
                              const char* moduleName,
                              void* opaque) {
  char fullPath[LOADER_PATH_MAX];

  // Try to find the file in any of the registered base folders
  for (int i = 0; i < qjsBaseFoldersCount; i++) {
    if (isAbsolutePath(moduleName) || getActualFilePath(moduleName)) {
      snprintf(fullPath, sizeof(fullPath), "%s", moduleName);
    } else {
      snprintf(fullPath, sizeof(fullPath), "%s/%s", qjsBaseFolders[i], moduleName);
    }

    const char* actualPath = getActualFilePath(fullPath);
    if (actualPath) { // file exists, it's a js file outside node_modules
      const JSValue funcObj = loadJsModule(ctx, moduleName);
      return JS_VALUE_GET_PTR(funcObj);
    }
  }

  // try to load the file from node_modules in the base folders
  char* nodeModuleEntryFile = tryFindNodeModuleEntryPath(moduleName);
  if (!nodeModuleEntryFile) {
    logError("Failed to load the js module: %s", moduleName);
    return NULL;
  }

  char modulePath[LOADER_PATH_MAX];
  snprintf(modulePath, sizeof(modulePath), "node_modules/%s/%s", moduleName, nodeModuleEntryFile);
  free(nodeModuleEntryFile);

  const JSValue funcObj = loadJsModule(ctx, modulePath);
  return JS_VALUE_GET_PTR(funcObj);
}
