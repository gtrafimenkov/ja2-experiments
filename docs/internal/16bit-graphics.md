# 16-bit graphics

The original game was programmed to use 16-bit graphics.
The game has to handle different modes (RGB565, RGB555, etc.) because
what modes were supported depended on the hardware (video card).

These modes are outdated now.  32bit mode should be used instead.

Maybe useful:
- https://www.gamedev.net/forums/topic/54104-555-or-565/
- https://learn.microsoft.com/en-us/windows/win32/directshow/working-with-16-bit-rgb

The problem with running a debug build is also related to 16-bit graphics.
To run a debug build it is necessary to create the game window not in full-screen
mode, but it is not possible on modern hardware.
