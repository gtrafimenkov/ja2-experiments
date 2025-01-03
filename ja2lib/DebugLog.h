#ifndef JA2_DEBUG_LOG_H
#define JA2_DEBUG_LOG_H

// Writes a simple string into file "debug-log-%Y%m%d-%H%M%S.txt".
// This function is not multithread safe.
void DebugLog(const char* message);

// Writes the formatted string into file "debug-log-%Y%m%d-%H%M%S.txt"
// This function is not multithread safe.
void DebugLogF(const char* format, ...);

#endif
