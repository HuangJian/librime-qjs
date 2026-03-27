#ifndef NODE_MODULE_LOADER_WIN_H
#define NODE_MODULE_LOADER_WIN_H

#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <windows.h>

typedef struct _stat StatStruct;
#define STAT_FUNC _stat

static int isAbsolutePath(const char* pathStr) {
  const size_t len = strlen(pathStr);
  if (len == 0) {
    return 0;
  }
  return pathStr[0] == '/' || pathStr[0] == '\\' ||
         (len > 1 && isalpha(pathStr[0]) && pathStr[1] == ':');
}

static int getExecutablePath(char* outPath, size_t size) {
  if (GetModuleFileNameA(NULL, (LPSTR)outPath, (DWORD)size) > 0) {
    return 0;
  }
  return -1;
}

#endif  // NODE_MODULE_LOADER_WIN_H
