// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __INTERFACE_CONTROL_H
#define __INTERFACE_CONTROL_H

#include "SGP/Types.h"

#define INTERFACE_MAPSCREEN 0x00000001
#define INTERFACE_NORENDERBUTTONS 0x00000002
#define INTERFACE_LOCKEDLEVEL1 0x00000004
#define INTERFACE_SHOPKEEP_INTERFACE 0x00000008

extern uint32_t guiTacticalInterfaceFlags;

void SetTacticalInterfaceFlags(uint32_t uiFlags);

void SetUpInterface();
void ResetInterface();
void RenderTopmostTacticalInterface();
void RenderTacticalInterface();

void StartViewportOverlays();
void EndViewportOverlays();

void LockTacticalInterface();
void UnLockTacticalInterface();

void RenderTacticalInterfaceWhileScrolling();

void EraseInterfaceMenus(BOOLEAN fIgnoreUIUnLock);

// handle paused render of tactical panel, if flag set, OR it in with tactical render flags
// then reset
void HandlePausedTacticalRender(void);

void ResetInterfaceAndUI();

BOOLEAN AreWeInAUIMenu();

void HandleTacticalPanelSwitch();

BOOLEAN InterfaceOKForMeanwhilePopup();

#endif
