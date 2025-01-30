#ifndef JA2_COMMAND_LINE_PARSER
#define JA2_COMMAND_LINE_PARSER

#include <stdbool.h>

typedef struct {
  bool help;
  bool nosound;
  char *datadir;

  char *errorMessage;
} CmdLineArgs;

void CmdLinePrintHelp(const char *programName);
bool CmdLineParse(int argc, char *argv[], CmdLineArgs *parsed);
void CmdLineFree(CmdLineArgs *parsed);

#endif  // JA2_COMMAND_LINE_PARSER
