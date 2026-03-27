#ifndef NODE_MODULE_LOADER_MAC_H
#define NODE_MODULE_LOADER_MAC_H

#include <mach-o/dyld.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct stat StatStruct;
#define STAT_FUNC stat

static int isAbsolutePath(const char* path) {
  return path && path[0] == '/';
}

static int getExecutablePath(char* path, const size_t size) {
  uint32_t macSize = (uint32_t)size;
  if (_NSGetExecutablePath(path, &macSize) == 0) {
    return 0;
  }
  return -1;
}

#endif  // NODE_MODULE_LOADER_MAC_H
