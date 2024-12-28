#pragma once

/* Warning, this file is autogenerated by cbindgen. Don't modify this manually. */

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

struct Str512 {
  char buf[512];
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Change the current working directory.
 * The path string must be utf8 encoded.
 */
bool Plat_SetCurrentDirectory(const char *path_utf8);

/**
 * Check if directory exists.
 */
bool Plat_DirectoryExists(const char *path_utf8);

/**
 * Check if file or directory exists.
 */
bool Plat_FileEntityExists(const char *path_utf8);

/**
 * Create directory.
 */
bool Plat_CreateDirectory(const char *path_utf8);

bool Plat_DeleteFile(const char *path_utf8);

/**
 * Remove all files in a directory, but not the directory itself or any subdirectories.
 */
bool Plat_RemoveFilesInDirectory(const char *path_utf8);

/**
 * Remove the directory and all its content including subdirectories.
 */
bool Plat_RemoveDirectory(const char *path_utf8);

/**
 * Remove read-only attribute from a file.
 */
bool Plat_RemoveReadOnlyAttribute(const char *path_utf8);

/**
 * Return the current working directory.
 */
bool Plat_GetCurrentDirectory(struct Str512 *dest);

/**
 * Return the directory where executable file is located.
 */
bool Plat_GetExecutableDirectory(struct Str512 *dest);

/**
 * Given a path, fill dest with the file name.
 */
bool Plat_FileBaseName(const char *path_utf8, struct Str512 *dest);

/**
 * Copy string "Foo" into the buffer.
 * If not enough space, return false and fill the buffer with zeroes.
 */
bool GetStrTest_Foo(char *buf, uintptr_t buf_size);

/**
 * Copy string "Привет" into the buffer.
 * The string will be utf-8 encoded.
 * If not enough space, return false and fill the buffer with zeroes.
 */
bool GetStrTest_HelloRus(char *buf, uintptr_t buf_size);

/**
 * Copy string "Hello String512" to str.
 */
bool GetStrTest_Hello512(struct Str512 *str);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
