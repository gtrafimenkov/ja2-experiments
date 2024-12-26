# Jagged Alliance 2 Vanilla in Rust

This is a rewrite of [ja2-vanilla](https://github.com/gtrafimenkov/ja2-vanilla) project
in Rust.

Project goals:
- full rewrite in Rust

Non-goals:
- adding new functionality
- supporting game mods

## Project structure

```
ja2lib             - platform-independent part of the game code
platfrom-dummy     - dummy implementation of the platfrom code (used in unit tests)
platform-linux     - platform code for Linux
platform-win32     - platform code for win32
bin-win32          - project to build the game binary for Windows
bin-linux          - project to build Linux binary of the game (not implemented)
rustlib            - Rust code compiled to a dynamic shared library
unittester         - an application to run unit tests
```

## Build requirements

- CMake
- GCC or Clang for Linux
- Visual Studio Community 2022 for Windows
- Rust v1.67.0 or later

## How to build test and run

```
python xx.py build test copy-data run
```

## How to play the game

- install the original version of the game (from the original game CDs, Steam, gog.com, etc.)
- `python xx.py build test copy-data run` or copy content of `build\bin-win32\RelWithDebInfo`
  folder to the game directory alongside the original ja2.exe and run `ja2v.exe`

The game is tested on Windows 10.

## License

This is not open source as defined by [The Open Source Definition](https://opensource.org/osd/).
The original JA2 sources were released under the terms of [SFI-SCLA](SFI-SCLA.txt) license.
Please read it carefully and make your own mind regarding this.
