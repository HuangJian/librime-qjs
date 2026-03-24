#ifndef NODE_MODULE_LOADER_OS_H
#define NODE_MODULE_LOADER_OS_H

#include <ctype.h>
#include <errno.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#include <sys/stat.h>
typedef struct _stat StatStruct;
#define STAT_FUNC _stat
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
typedef struct stat StatStruct;
#define STAT_FUNC stat
#elif defined(__linux__)
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
typedef struct stat StatStruct;
#define STAT_FUNC stat
#else
#include <sys/stat.h>
#include <unistd.h>
typedef struct stat StatStruct;
#define STAT_FUNC stat
#endif

// If the system doesn't provide S_ISDIR / S_ISREG macros, define them manually.
#ifndef S_ISDIR
#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(m) (((m)&S_IFMT) == S_IFREG)
#endif

typedef enum {
  FILE_TYPE_ERROR = -1,    // error occurred while checking the file type
  FILE_TYPE_NOT_EXIST = 0, // path is not exist
  FILE_TYPE_REG = 1,       // regular file
  FILE_TYPE_DIR = 2,       // directory
  FILE_TYPE_OTHER = 3      // other type (like device, pipe, etc.)
} FileType;

static inline int osIsAbsolutePath(const char* path) {
  const size_t len = strlen(path);
  if (len == 0) return 0;
#if defined(_WIN32)
  return (len > 1 && isalpha(path[0]) && path[1] == ':') ||
         (len > 1 && path[0] == '\\' && path[1] == '\\');
#else
  return path[0] == '/';
#endif
}

static inline int osGetExecutablePath(char* path, size_t size) {
#if defined(_WIN32)
  if (GetModuleFileNameA(NULL, path, (DWORD)size) > 0) return 0;
#elif defined(__APPLE__)
  uint32_t apple_size = (uint32_t)size;
  if (_NSGetExecutablePath(path, &apple_size) == 0) return 0;
#elif defined(__linux__)
  ssize_t count = readlink("/proc/self/exe", path, size - 1);
  if (count != -1) {
    path[count] = '\0';
    return 0;
  }
#endif
  return -1;
}

#endif  // NODE_MODULE_LOADER_OS_H
