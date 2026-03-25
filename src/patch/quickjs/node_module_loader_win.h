#ifndef NODE_MODULE_LOADER_WIN_H
#define NODE_MODULE_LOADER_WIN_H

#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <windows.h>

typedef struct _stat StatStruct;
#define STAT_FUNC _stat

static int isAbsolutePath(const char* path) {
  const size_t len = strlen(path);
  if (len == 0) {
    return 0;
  }
  return path[0] == '/' || path[0] == '\\' || (len > 1 && isalpha(path[0]) != 0 && path[1] == ':');
}

static int getExecutablePath(char* path, size_t size) {  // NOLINT(*-non-const-parameter)
  if (GetModuleFileNameA(NULL, path, (DWORD)size) > 0) {
    return 0;
  }
  return -1;
}

#endif  // NODE_MODULE_LOADER_WIN_H
