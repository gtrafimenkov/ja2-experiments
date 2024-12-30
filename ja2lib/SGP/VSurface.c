// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#include "VSurface.h"

#include "SGP/Debug.h"
#include "SGP/VObject.h"
#include "StrUtils.h"

bool AddVSurfaceFromFile(const char *filepath, VSurfID *index) {
  return AddVSurface(CreateVSurfaceFromFile(filepath), index);
}

typedef struct VSURFACE_NODE {
  struct VSurface *hVSurface;
  uint32_t uiIndex;
  struct VSURFACE_NODE *next, *prev;
} VSURFACE_NODE;

VSURFACE_NODE *gpVSurfaceHead = NULL;
VSURFACE_NODE *gpVSurfaceTail = NULL;
uint32_t guiVSurfaceIndex = 0;
uint32_t guiVSurfaceSize = 0;
uint32_t guiVSurfaceTotalAdded = 0;

void InitVSurfaceList() {
  Assert(!gpVSurfaceHead);
  Assert(!gpVSurfaceTail);
  gpVSurfaceHead = gpVSurfaceTail = NULL;
}

void DeinitVSurfaceList() {
  while (gpVSurfaceHead) {
    VSURFACE_NODE *curr = gpVSurfaceHead;
    gpVSurfaceHead = gpVSurfaceHead->next;
    DeleteVideoSurface(curr->hVSurface);
    MemFree(curr);
  }
  gpVSurfaceHead = NULL;
  gpVSurfaceTail = NULL;
  guiVSurfaceIndex = 0;
  guiVSurfaceSize = 0;
  guiVSurfaceTotalAdded = 0;
}

VSurfID AddVSurfaceToList(struct VSurface *vs) {
  // Set into video object list
  if (gpVSurfaceHead) {
    // Add node after tail
    gpVSurfaceTail->next = (VSURFACE_NODE *)MemAlloc(sizeof(VSURFACE_NODE));
    Assert(gpVSurfaceTail->next);  // out of memory?
    gpVSurfaceTail->next->prev = gpVSurfaceTail;
    gpVSurfaceTail->next->next = NULL;
    gpVSurfaceTail = gpVSurfaceTail->next;
  } else {
    // new list
    gpVSurfaceHead = (VSURFACE_NODE *)MemAlloc(sizeof(VSURFACE_NODE));
    Assert(gpVSurfaceHead);  // out of memory?
    gpVSurfaceHead->prev = gpVSurfaceHead->next = NULL;
    gpVSurfaceTail = gpVSurfaceHead;
  }
  // Set the hVSurface into the node.
  gpVSurfaceTail->hVSurface = vs;
  gpVSurfaceTail->uiIndex = guiVSurfaceIndex += 2;
  Assert(guiVSurfaceIndex < 0xfffffff0);  // unlikely that we will ever use 2 billion VSurfaces!
  // We would have to create about 70 VSurfaces per second for 1 year straight to achieve this...
  guiVSurfaceSize++;
  guiVSurfaceTotalAdded++;
  return gpVSurfaceTail->uiIndex;
}

struct VSurface *FindVSurface(VSurfID id) {
  VSURFACE_NODE *curr = gpVSurfaceHead;
  while (curr) {
    if (curr->uiIndex == id) {
      return curr->hVSurface;
    }
    curr = curr->next;
  }
  return NULL;
}

BOOLEAN RestoreVideoSurfaces() {
  VSURFACE_NODE *curr = gpVSurfaceTail;
  while (curr) {
    if (!RestoreVideoSurface(curr->hVSurface)) {
      return FALSE;
    }
    curr = curr->prev;
  }
  return TRUE;
}

bool DeleteVSurfaceFromList(VSurfID id) {
  VSURFACE_NODE *curr = gpVSurfaceHead;
  while (curr) {
    if (curr->uiIndex == id) {  // Found the node, so detach it and delete it.

      // Deallocate the memory for the video surface
      DeleteVideoSurface(curr->hVSurface);

      if (curr ==
          gpVSurfaceHead) {  // Advance the head, because we are going to remove the head node.
        gpVSurfaceHead = gpVSurfaceHead->next;
      }
      if (curr ==
          gpVSurfaceTail) {  // Back up the tail, because we are going to remove the tail node.
        gpVSurfaceTail = gpVSurfaceTail->prev;
      }
      // Detach the node from the vsurface list
      if (curr->next) {  // Make the prev node point to the next
        curr->next->prev = curr->prev;
      }
      if (curr->prev) {  // Make the next node point to the prev
        curr->prev->next = curr->next;
      }
      // The node is now detached.  Now deallocate it.
      MemFree(curr);
      guiVSurfaceSize--;
      return true;
    }
    curr = curr->next;
  }
  return false;
}

BOOLEAN AddVSurface(struct VSurface *vs, uint32_t *puiIndex) {
  if (!vs) {
    return FALSE;
  }

  // Set transparency to default
  SetVideoSurfaceTransparencyColor(vs, FROMRGB(0, 0, 0));

  *puiIndex = AddVSurfaceToList(vs);

  return TRUE;
}

struct VSurface *CreateVSurfaceBlank8(uint16_t width, uint16_t height) {
  return CreateVSurfaceBlank(width, height, 8);
}

struct VSurface *CreateVSurfaceBlank16(uint16_t width, uint16_t height) {
  return CreateVSurfaceBlank(width, height, 16);
}
