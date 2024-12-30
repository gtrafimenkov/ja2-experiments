// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "VSurface.h"

#include "StrUtils.h"

struct VSurface *CreateVideoSurfaceFromFile(char *filepath) {
  VSURFACE_DESC desc;
  desc.fCreateFlags = VSURFACE_CREATE_FROMFILE;
  strcopy(desc.ImageFile, sizeof(desc.ImageFile), filepath);
  return CreateVideoSurface(&desc);
}

bool AddVideoSurfaceFromFile(const char *filepath, uint32_t *index) {
  VSURFACE_DESC desc;
  desc.fCreateFlags = VSURFACE_CREATE_FROMFILE;
  strcopy(desc.ImageFile, sizeof(desc.ImageFile), filepath);
  return AddVideoSurface(&desc, index);
}
