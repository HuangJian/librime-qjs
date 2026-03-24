#ifndef NODE_MODULE_LOADER_LINUX_H
#define NODE_MODULE_LOADER_LINUX_H

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
  ssize_t count = readlink("/proc/self/exe", path, size - 1);
  if (count != -1) {
    path[count] = '\0';
    return 0;
  }
  return -1;
}

#endif  // NODE_MODULE_LOADER_LINUX_H
