#ifndef NODE_MODULE_LOADER_MAC_H
#define NODE_MODULE_LOADER_MAC_H

#include <mach-o/dyld.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

typedef struct stat StatStruct;
#define STAT_FUNC stat

static inline int osIsAbsolutePath(const char* path) {
  return path && path[0] == '/';
}

static inline int osGetExecutablePath(char* path, size_t size) {
  uint32_t mac_size = (uint32_t)size;
  if (_NSGetExecutablePath(path, &mac_size) == 0) return 0;
  return -1;
}

#endif  // NODE_MODULE_LOADER_MAC_H
