#include "DebugLog.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static FILE* logFile = NULL;
static char localtime_str[20];
static char message_buf[1024];

const char* getLocalTime() {
  time_t now;
  struct tm local_time;
  memset(&local_time, 0, sizeof(local_time));

  time(&now);
  localtime_s(&local_time, &now);
  strftime(localtime_str, sizeof(localtime_str), "%Y%m%d-%H%M%S", &local_time);

  return localtime_str;
}

static bool openFile(const char* localtime_str) {
  if (logFile == NULL) {
    char filename[32];
    sprintf_s(filename, sizeof(filename), "debug-log-%s.txt", localtime_str);
    if (fopen_s(&logFile, filename, "a") != 0) {
      fprintf(stderr, "Error opening log file '%s'.\n", filename);
      return false;
    }
  }
  return true;
}

void DebugLog(const char* message) {
  const char* localtime_str = getLocalTime();
  if (openFile(localtime_str)) {
    fprintf(logFile, "%s: %s\n", localtime_str, message);
    fflush(logFile);
  }
}

void DebugLogF(const char* format, ...) {
  const char* localtime_str = getLocalTime();
  if (!openFile(localtime_str)) {
    return;
  }

  va_list args;
  va_start(args, format);
  size_t written = vsprintf_s(message_buf, sizeof(message_buf) - 1, format, args);
  message_buf[written] = 0;
  va_end(args);

  fprintf(logFile, "%s: %s\n", localtime_str, message_buf);

  fflush(logFile);
}
