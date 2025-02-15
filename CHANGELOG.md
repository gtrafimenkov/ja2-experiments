# List of significant changes

## 2025-02

- Direct Draw 2 drawing code is abstracted behind jplatform_video.h interface, making it
  easier to switch to another graphics implementation, for example, SDL2

## 2025-01

- automatic detection of game resources type (English, French, Russian Buka, Russian Gold, etc.)
- single build supports all languages; text language is selected dynamically to match
  the game data (e.g. Russian text for Russian game resources)
