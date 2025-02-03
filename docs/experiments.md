# List of possible experiments

## Swappable platform layer

Game code is separated from the code that deal with the OS (platform).  Platform
layer abstracts away all the platform differences through common interface.
It is possible to swap the platform layer at the compilation time.

Platform layers:
- DirectDraw 2 (original JA2 implementation)
- SDL 2.0
- SDL 3.0

## Input recording and playback

All inputs events are recorded and save.  It is possible to use the recorded
events instead of actual input.  This way it will be possible to test the game
by playing back old recordings.

## Using memory arena technique instead of malloc/free
