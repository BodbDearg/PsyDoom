# StationDoom
This project is a reverse engineering attempt to backport PSX Doom to PC. Will add more details here later and eventually binary builds once it is stable enough for general release. If you want to try/experiment with this for now, you will need to build from source - see details below.

As of right now the game mostly runs correctly, with some sound syncing issues and a few other problems. Here is a brief video demonstration showing the project in action:

[![Alt text](https://img.youtube.com/vi/o7t7w1YjjSw/0.jpg)](https://www.youtube.com/watch?v=o7t7w1YjjSw)

The eventual goal of this project is convert the entire game code to C++, remove the PSX BIOS dependency and need for the original .EXE, and hopefully also simplify + remove most emulation code except where strictly necessary for authenticity. All of this while preserving as much of the original code structure and meaning as possible, for historical reference..

Longer term goals include support for Final Doom, proper modding support (I added some basic support so far!) and PSX DOOM engine limit removal. Once all those things have been done this project can also be forked to provide an 'enhanced' version of game with support for higher resolution, fixing of graphical glitches, support for widescreen and so on.

## How to build
- Requires a recent CMake to generate the project (3.14 or higher)
- Builds with Visual Studio 2017/2019 (Windows 64-bit) and also Xcode 11 on MacOS. Other platforms & IDEs not yet supported/tested.

## Running
- The following items must be present in the working directory of the application:
  - **SCPH1001.BIN**: The original US/NTSC PlayStation 1 BIOS. Later US/NTSC bioses may also work if aliased to this name.
  - **PSXDOOM.EXE**: The PSX DOOM game .EXE file as extracted from the game disc. *MUST* be the US/NTSC version of PlayStation DOOM, or product SLUS-00077).
  - **DOOM.CUE**: A .cue file pointing to all of the tracks and binary data for the game disc. The disc *MUST* be the US/NTSC version of PlayStation DOOM, or product SLUS-00077).

## Current limitations/bugs
- Sound is frequently out of sync with gameplay - not severely but sometimes it can be noticed. Probably due to emulation being either advanced too much or too little. Should eventually be fixed once sound engine advancement is tied to real-time.
- Occasionally the game will freeze when loading maps.
- The Avocado PSX emulator backend (used for hardware emulation and bios calls) seems to draw some sprites 1 pixel too small in some cases.
- Only the US version of PSX DOOM is supported, not Final DOOM or any other SKU.
- Multiplayer does not work and will freeze the game.
- The intro movie does not play yet.
- Probably other lots of other stuff not mentioned here...
