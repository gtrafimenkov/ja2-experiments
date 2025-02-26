// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef __TACTICAL_SAVE_H_
#define __TACTICAL_SAVE_H_

#include "SGP/Types.h"
#include "Tactical/RottingCorpses.h"
#include "Tactical/SoldierProfileType.h"
#include "Tactical/WorldItems.h"

#define MAPS_DIR "Temp\\"

// Defines used for the bUseMercGridNoPlacement contained in the the merc profile struct
enum {
  PROFILE_NOT_SET,          // initially set to this
  PROFILE_DONT_USE_GRIDNO,  // if the merc is switching sectors, etc
  PROFILE_USE_GRIDNO,       // if we are to use the GridNo variable in the profile struct
};

// Add
BOOLEAN AddMapModification(uint8_t sMapX, uint8_t sMapY, int8_t bMapZ);

// Load the Map modifications from the saved game file
BOOLEAN LoadMapTempFilesFromSavedGameFile(HWFILE hFile);

// Save the Map Temp files to the saved game file
BOOLEAN SaveMapTempFilesToSavedGameFile(HWFILE hFile);

// delete temp file
BOOLEAN DeleteTempItemMapFile(uint8_t sMapX, uint8_t sMapY, int8_t bMapZ);

// Retrieves the number of items in the sectors temp item file
BOOLEAN GetNumberOfWorldItemsFromTempItemFile(uint8_t sMapX, uint8_t sMapY, int8_t bMapZ,
                                              uint32_t *puiNumberOfItems, BOOLEAN fIfEmptyCreate);

// Saves the Current Sectors, ( world Items, rotting corpses, ... )  to the temporary file used to
// store the sectors items
BOOLEAN SaveCurrentSectorsInformationToTempItemFile();

// Loads the Currents Sectors information ( world Items, rotting corpses, ... ) from the temporary
// file used to store the sectores items
BOOLEAN LoadCurrentSectorsInformationFromTempItemsFile();

// Loads a World Item array from that sectors temp item file
BOOLEAN LoadWorldItemsFromTempItemFile(uint8_t sMapX, uint8_t sMapY, int8_t bMapZ,
                                       WORLDITEM *pData);

//  Adds an array of Item Objects to the specified location on a unloaded map.
//  If you want to overwrite all the items in the array set fReplaceEntireFile to TRUE.
BOOLEAN AddItemsToUnLoadedSector(uint8_t sMapX, uint8_t sMapY, int8_t bMapZ, int16_t sGridNo,
                                 uint32_t uiNumberOfItems, struct OBJECTTYPE *pObject,
                                 uint8_t ubLevel, uint16_t usFlags, int8_t bRenderZHeightAboveLevel,
                                 int8_t bVisible, BOOLEAN fReplaceEntireFile);

BOOLEAN AddWorldItemsToUnLoadedSector(uint8_t sMapX, uint8_t sMapY, int8_t bMapZ, int16_t sGridNo,
                                      uint32_t uiNumberOfItems, WORLDITEM *pWorldItem,
                                      BOOLEAN fOverWrite);

// Deletes all the Temp files in the Maps\Temp directory
BOOLEAN InitTacticalSave(BOOLEAN fCreateTempDir);

// Gets the number of ACTIVE ( Not the TOTAL number ) of World Items from the sectors temp file
BOOLEAN GetNumberOfActiveWorldItemsFromTempFile(uint8_t sMapX, uint8_t sMapY, int8_t bMapZ,
                                                uint32_t *pNumberOfData);

// Call this function to set the new sector a NPC will travel to
void ChangeNpcToDifferentSector(uint8_t ubNpcId, uint8_t sSectorX, uint8_t sSectorY,
                                int8_t bSectorZ);

// Adds a rotting corpse definition to the end of a sectors rotting corpse temp file
BOOLEAN AddRottingCorpseToUnloadedSectorsRottingCorpseFile(
    uint8_t sMapX, uint8_t sMapY, int8_t bMapZ, ROTTING_CORPSE_DEFINITION *pRottingCorpseDef);

// Flags used for the AddDeadSoldierToUnLoadedSector() function
#define ADD_DEAD_SOLDIER_USE_GRIDNO \
  0x00000001  // just place the items and corpse on the gridno location
#define ADD_DEAD_SOLDIER_TO_SWEETSPOT 0x00000002  // Finds the closet free gridno

#define ADD_DEAD_SOLDIER__USE_JFK_HEADSHOT_CORPSE 0x00000040  // Will ue the JFK headshot

// Pass in the sector to add the dead soldier to.
// The gridno if you are passing in either of the flags ADD_DEAD_SOLDIER_USE_GRIDNO, or the
// ADD_DEAD_SOLDIER_TO_SWEETSPOT
//
// This function DOES NOT remove the soldier from the soldier struct.  YOU must do it.
BOOLEAN AddDeadSoldierToUnLoadedSector(uint8_t sMapX, uint8_t sMapY, uint8_t bMapZ,
                                       struct SOLDIERTYPE *pSoldier, int16_t sGridNo,
                                       uint32_t uiFlags);

BOOLEAN GetSectorFlagStatus(uint8_t sMapX, uint8_t sMapY, uint8_t bMapZ, uint32_t uiFlagToSet);
BOOLEAN SetSectorFlag(uint8_t sMapX, uint8_t sMapY, uint8_t bMapZ, uint32_t uiFlagToSet);
BOOLEAN ReSetUnderGroundSectorFlag(uint8_t sSectorX, uint8_t sSectorY, uint8_t ubSectorZ,
                                   uint32_t uiFlagToSet);
BOOLEAN ReSetSectorFlag(uint8_t sMapX, uint8_t sMapY, uint8_t bMapZ, uint32_t uiFlagToSet);

// Saves the NPC temp Quote file to the saved game file
BOOLEAN LoadTempNpcQuoteArrayToSaveGameFile(HWFILE hFile);

// Loads the NPC temp Quote file from the saved game file
BOOLEAN SaveTempNpcQuoteArrayToSaveGameFile(HWFILE hFile);

uint32_t MercChecksum(struct SOLDIERTYPE *pSoldier);
uint32_t ProfileChecksum(MERCPROFILESTRUCT *pProfile);
BOOLEAN JA2EncryptedFileRead(HWFILE hFile, void *pDest, uint32_t uiBytesToRead,
                             uint32_t *puiBytesRead);
BOOLEAN JA2EncryptedFileWrite(HWFILE hFile, void *pDest, uint32_t uiBytesToWrite,
                              uint32_t *puiBytesWritten);

BOOLEAN NewJA2EncryptedFileRead(HWFILE hFile, void *pDest, uint32_t uiBytesToRead,
                                uint32_t *puiBytesRead);
BOOLEAN NewJA2EncryptedFileWrite(HWFILE hFile, void *pDest, uint32_t uiBytesToWrite,
                                 uint32_t *puiBytesWritten);

// If hacker's mess with our save/temp files, this is our final line of defence.
void InitExitGameDialogBecauseFileHackDetected();

void HandleAllReachAbleItemsInTheSector(uint8_t sSectorX, uint8_t sSectorY, int8_t bSectorZ);

void GetMapTempFileName(uint32_t uiType, char *pMapName, uint8_t sMapX, uint8_t sMapY,
                        int8_t bMapZ);

uint32_t GetNumberOfVisibleWorldItemsFromSectorStructureForSector(uint8_t sMapX, uint8_t sMapY,
                                                                  int8_t bMapZ);
void SetNumberOfVisibleWorldItemsInSectorStructureForSector(uint8_t sMapX, uint8_t sMapY,
                                                            int8_t bMapZ, uint32_t uiNumberOfItems);

#define NEW_ROTATION_ARRAY_SIZE 49
#define BASE_NUMBER_OF_ROTATION_ARRAYS 19

#endif
