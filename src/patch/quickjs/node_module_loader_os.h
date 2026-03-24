#ifndef NODE_MODULE_LOADER_OS_H
#define NODE_MODULE_LOADER_OS_H

#include <errno.h>

#if defined(_WIN32)
#include "node_module_loader_win.h"
#elif defined(__APPLE__)
#include "node_module_loader_mac.h"
#elif defined(__linux__)
#include "node_module_loader_linux.h"
#else
#include <unistd.h>
#include <sys/stat.h>
typedef struct stat StatStruct;
#define STAT_FUNC stat
static inline int osIsAbsolutePath(const char* path) { return path && path[0] == '/'; }
static inline int osGetExecutablePath(char* path, size_t size) { return -1; }
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

#endif  // NODE_MODULE_LOADER_OS_H
