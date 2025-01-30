#include "CmdLine.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Alloc.h"
#include "DebugLog.h"

void doublePrintF(const char *format, ...) {
  va_list args;
  va_start(args, format);

  char buf[200];
  vsnprintf(buf, 200, format, args);
  va_end(args);

  fprintf(stderr, "%s\n", buf);
  DebugLog(buf);
}

void CmdLinePrintHelp(const char *programName) {
  doublePrintF("Usage: %s [OPTIONS]", programName);
  doublePrintF("Options:");
  doublePrintF("  --help          Show this help message and exit");
  doublePrintF("  --nosound       Disable sound");
  doublePrintF("  --datadir PATH  Set data directory path");
}

bool CmdLineParse(int argc, char *argv[], CmdLineArgs *parsed) {
  MemZero(parsed, sizeof(CmdLineArgs));
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0) {
      parsed->help = true;
    } else if (strcmp(argv[i], "--nosound") == 0) {
      parsed->nosound = true;
    } else if (strcmp(argv[i], "--datadir") == 0) {
      if (i + 1 < argc) {
        parsed->datadir = argv[i + 1];
        i++;
      } else {
        parsed->errorMessage = strdup("Error: --datadir requires a path");
        return false;
      }
    } else {
      my_asprintf(&parsed->errorMessage, "Unknown option: %s", argv[i]);
      return false;
    }
  }
  return true;
}

void CmdLineFree(CmdLineArgs *parsed) {
  MemFree(parsed->datadir);
  MemFree(parsed->errorMessage);
  MemZero(parsed, sizeof(CmdLineArgs));
}
