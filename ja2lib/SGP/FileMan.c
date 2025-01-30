// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "SGP/FileMan.h"

#include <stdio.h>

#include "DebugLog.h"
#include "MemMan.h"
#include "SGP/Debug.h"
#include "SGP/LibraryDataBase.h"
#include "platform.h"

// TODO: all FileMan_ functions implementations should go here

BOOLEAN FileMan_Initialize() {
  RegisterDebugTopic(TOPIC_FILE_MANAGER, "File Manager");
  return (TRUE);
}

void FileMan_Shutdown(void) { UnRegisterDebugTopic(TOPIC_FILE_MANAGER, "File Manager"); }

BOOLEAN FileMan_ExistsNoDB(char* strFilename) {
  BOOLEAN fExists = FALSE;
  FILE* file;

  // open up the file to see if it exists on the disk
  file = fopen(strFilename, "r");
  if (file) {
    fExists = TRUE;
    fclose(file);
  }

  return (fExists);
}

BOOLEAN FileMan_Delete(char* strFilename) { return (Plat_DeleteFile(strFilename)); }

BOOLEAN FileMan_Exists(char* strFilename) {
  BOOLEAN fExists = FALSE;
  FILE* file;

  // open up the file to see if it exists on the disk
  file = fopen(strFilename, "r");
  if (file) {
    fExists = TRUE;
    fclose(file);
  }

  // if the file wasnt on disk, check to see if its in a library
  if (fExists == FALSE) {
    fExists = CheckIfFileExistInLibrary(strFilename);
  }

  return (fExists);
}

bool FileMan_AllocReadFullFile(char* filename, void** data, uint32_t* size) {
  HWFILE f = FileMan_Open(filename, FILE_ACCESS_READ, FALSE);
  if (!f) {
    return false;
  }

  *size = FileMan_GetSize(f);

  *data = MemAlloc(*size);
  if (!*data) {
    FileMan_Close(f);
    return false;
  }

  uint32_t bytesRead;
  if (!FileMan_Read(f, *data, *size, &bytesRead) || bytesRead != *size) {
    FileMan_Close(f);
    MemFree(*data);
    *data = NULL;
    return false;
  }

  FileMan_Close(f);
  return true;
}

bool FileMan_CalcSHA256(char* filename, SHA256STR strhash) {
  HWFILE f = FileMan_Open(filename, FILE_ACCESS_READ, FALSE);
  if (!f) {
    return false;
  }

  SHA256_CTX ctx;
  sha256_init(&ctx);

  uint8_t buf[4096];
  uint32_t bytesLeft = FileMan_GetSize(f);
  while (bytesLeft > 0) {
    uint32_t readBytes = 0;
    uint32_t bytesToRead = min(bytesLeft, 4096);
    if (!FileMan_Read(f, &buf, bytesToRead, &readBytes)) {
      FileMan_Close(f);
      return false;
    }
    sha256_update(&ctx, buf, readBytes);
    bytesLeft -= readBytes;
  }
  FileMan_Close(f);

  SHA256 hash;
  sha256_final(&ctx, hash);
  sha256_to_string(hash, strhash);

  return true;
}
