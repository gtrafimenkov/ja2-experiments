#ifndef JPLATFORM_VIDEO_WINDD2_H
#define JPLATFORM_VIDEO_WINDD2_H

#include <stdint.h>

// JVideoInitParams for DirectDraw 2 platform.
struct JVideoInitParams {
  void* hInstance;
  uint16_t usCommandShow;
  void* WindowProc;
  uint16_t iconID;
};

#endif  // JPLATFORM_VIDEO_WINDD2_H
