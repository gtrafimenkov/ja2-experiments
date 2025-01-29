#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool Plat_FindFilesWithExtCaseIns(const char *extension, void (*callback)(const char *)) {
  struct dirent *entry;
  DIR *dir = opendir(".");

  if (dir == NULL) {
    return false;
  }

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_REG) {  // Only regular files
      size_t len_filename = strlen(entry->d_name);
      size_t len_extension = strlen(extension);
      if (len_filename >= len_extension &&
          strcmp(entry->d_name + len_filename - len_extension, extension) == 0) {
        callback(entry->d_name);
      }
    }
  }

  closedir(dir);
  return true;
}
