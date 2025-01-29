#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "platform_strings.h"

bool Plat_FindFilesWithExtCaseIns(const char *extension, void (*callback)(const char *)) {
  WIN32_FIND_DATA findFileData;
  HANDLE hFind = FindFirstFile("./*", &findFileData);

  if (hFind == INVALID_HANDLE_VALUE) {
    return false;
  }

  do {
    if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      size_t len_filename = strlen(findFileData.cFileName);
      size_t len_extension = strlen(extension);
      if (len_filename >= len_extension &&
          strcasecmp(findFileData.cFileName + len_filename - len_extension, extension) == 0) {
        callback(findFileData.cFileName);
      }
    }
  } while (FindNextFile(hFind, &findFileData) != 0);

  FindClose(hFind);
  return true;
}
