#ifndef NODE_MODULE_LOADER_LINUX_H
#define NODE_MODULE_LOADER_LINUX_H

#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct stat StatStruct;
#define STAT_FUNC stat

static int isAbsolutePath(const char* path) {
  return path && path[0] == '/';
}

static int getExecutablePath(char* path, const size_t size) {
  const ssize_t count = readlink("/proc/self/exe", path, size - 1);
  if (count != -1) {
    path[count] = '\0';
    return 0;
  }
  return -1;
}

#endif  // NODE_MODULE_LOADER_LINUX_H
