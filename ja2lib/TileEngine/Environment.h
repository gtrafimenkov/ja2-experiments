// This is not free software.
// This file contains code derived from the code released under the terms
// of Strategy First Inc. Source Code License Agreement. See SFI-SCLA.txt.

#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include "SGP/Types.h"

#define ENV_TIME_00 0
#define ENV_TIME_01 1
#define ENV_TIME_02 2
#define ENV_TIME_03 3
#define ENV_TIME_04 4
#define ENV_TIME_05 5
#define ENV_TIME_06 6
#define ENV_TIME_07 7
#define ENV_TIME_08 8
#define ENV_TIME_09 9
#define ENV_TIME_10 10
#define ENV_TIME_11 11
#define ENV_TIME_12 12
#define ENV_TIME_13 13
#define ENV_TIME_14 14
#define ENV_TIME_15 15
#define ENV_TIME_16 16
#define ENV_TIME_17 17
#define ENV_TIME_18 18
#define ENV_TIME_19 19
#define ENV_TIME_20 20
#define ENV_TIME_21 21
#define ENV_TIME_22 22
#define ENV_TIME_23 23

#define ENV_NUM_TIMES 24

// Make sure you use 24 for end time hours and 0 for start time hours if
// midnight is the hour you wish to use.
#define NIGHT_TIME_LIGHT_START_HOUR 21
#define NIGHT_TIME_LIGHT_END_HOUR 7
#define PRIME_TIME_LIGHT_START_HOUR 21
#define PRIME_TIME_LIGHT_END_HOUR 24

#define WEATHER_FORECAST_SUNNY 0x00000001
#define WEATHER_FORECAST_OVERCAST 0x00000002
#define WEATHER_FORECAST_PARTLYSUNNY 0x00000004
#define WEATHER_FORECAST_DRIZZLE 0x00000008
#define WEATHER_FORECAST_SHOWERS 0x00000010
#define WEATHER_FORECAST_THUNDERSHOWERS 0x00000020

// higher is darker, remember
#define NORMAL_LIGHTLEVEL_NIGHT 12
#define NORMAL_LIGHTLEVEL_DAY 3

void ForecastDayEvents();

void EnvironmentController(BOOLEAN fCheckForLights);
void EnvEnableTOD(void);
void EnvDisableTOD(void);

void BuildDayAmbientSounds();
void BuildDayLightLevels();
uint8_t GetTimeOfDayAmbientLightLevel();

void EnvBeginRainStorm(uint8_t ubIntensity);
void EnvEndRainStorm();

extern uint8_t gubEnvLightValue;
extern BOOLEAN gfDoLighting;
extern uint32_t guiEnvWeather;

void TurnOnNightLights();
void TurnOffNightLights();
void TurnOnPrimeLights();
void TurnOffPrimeLights();

// effects whether or not time of day effects the lighting.  Underground
// maps have an ambient light level that is saved in the map, and doesn't change.
extern BOOLEAN gfCaves;
extern BOOLEAN gfBasement;

extern int8_t SectorTemperature(uint32_t uiTime, uint8_t sSectorX, uint8_t sSectorY,
                                int8_t bSectorZ);

extern void UpdateTemperature(uint8_t ubTemperatureCode);

#endif
