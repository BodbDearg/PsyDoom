# PsyDoom
*(Note: formerly known as 'StationDoom')*

This project is a reverse engineering attempt to backport PSX Doom to PC. The code is derived directly from the original machine code (see commit history for a timeline of its transformation) and currently runs in a semi-native, semi-emulated environment. The 'Avocado' PlayStation emulator is used to handle the specifics of the PSX hardware and emulate couple of small functions that don't yet work correctly in C++. Eventually the goal is reach 100% native status and remove all dependencies on emulation entirely.

A sister project, [PSXDOOM-RE](https://github.com/Erick194/PSXDOOM-RE), by [Erick Vásquez García (Erick194)](https://github.com/Erick194) also completely recreates the Doom source code for the actual PlayStation hardware and 'PsyQ' SDK. The reverse engineering work in that project is used to accelerate the incremental transition to native C++ for this project, and also interpretation of the code. Going forward, both projects will likely share improvements and refinements and collaborate even more.

Will add more details here later and eventually 'official' binary builds once it is stable enough for general release. If you want to try/experiment with this for now, you will need to build from source or use one of the occasional binaries that I will put up - see details below.

As of right now the game mostly runs correctly, with some sound syncing issues and a few other problems. Here is a brief video demonstration showing the project in action:

[![Alt text](https://img.youtube.com/vi/o7t7w1YjjSw/0.jpg)](https://www.youtube.com/watch?v=o7t7w1YjjSw)

As mentioned above the eventual goal of this project is convert the entire game code to C++, remove the PSX BIOS dependency and need for the original .EXE, and hopefully also simplify + remove most emulation code except where strictly necessary for authenticity. All of this while preserving as much of the original code structure and meaning as possible, for historical reference..

Longer term goals include support for Final Doom, proper modding support (I added some basic support so far!) and PSX DOOM engine limit removal. Once all those things have been done this project can also be forked to provide an 'enhanced' version of game with support for higher resolution, fixing of graphical glitches, support for widescreen and so on.

## How to build
- Requires a recent CMake to generate the project (3.14 or higher)
- Builds with Visual Studio 2017/2019 (Windows 64-bit) and also Xcode 11 on MacOS. Other platforms & IDEs not yet supported/tested.

## Running
- Make sure to have the latest Visual C redistributable installed:
  - https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads
- The following items must be present in the working directory of the application:
  - **SCPH1001.BIN**: The original US/NTSC PlayStation 1 BIOS. Later US/NTSC bioses like 'SCPH7001.BIN' may also work if aliased to this name.
  - **PSXDOOM.EXE**: The PSX DOOM game .EXE file as extracted from the game disc. 
    - This *MUST* be the 'Greatest Hits' US/NTSC version of PlayStation DOOM, or product SLUS-00077.
        - No other versions of the .EXE will work - the game relies on this exact .EXE layout!
    - The file size *MUST* be `428,032 bytes`.
    - The MD5 for the file *MUST* be: `fc9a10f36e6a4f6d5933c13ab77d3b06`
  - **DOOM.CUE**: A .cue file pointing to all of the tracks and binary data for the game disc. 
    - The disc *MUST* be the 'Greatest Hits' US/NTSC version of PlayStation DOOM, or product SLUS-00077.
        - No other versions of the CD will work - the game relies on this exact disc layout!
- PSX controls are currently hardcoded to be mapped to the keyboard as follows:
    - Up/down/left/right: Up/down/left/right
    - L1: A
    - R1: D
    - L2: Page Down or Q
    - R2: Page Up or E
    - Circle: Space
    - Square: Left or right shift
    - Triangle: Left or right CTRL, or F
    - Cross: Left or right ALT
    - Start: Return
    - Select: Tab
- In current dev builds, cheats can be activated easily by pressing the following function keys on the 'pause' screen:
    - F1: God mode
    - F2: Noclip (new cheat for PC port!)
    - F3: All weapons keys and ammo
    - F4: Level warp (note: secret maps can now be warped to also)
    - F5: XRay vision
    - F6: Show all map things
    - F7: Show all map lines
    - F8: VRAM Viewer (functionality hidden in retail)
- Hacky/temp 'mods' system.
    - You can override some files (currently .WAD and .IMG files only) by supplying the game with a directory containing those overrides.
    - Specify the directory using the `-datadir <MY_DIRECTORY_PATH>` command line argument.
    - Put files in this folder (note: not in any child folders!) that you wish to override, e.g 'MAP01.WAD'.
    - If the game goes to load a file such as 'MAP01.WAD' and it is present in the overrides dir, then the on-disk version will be used instead.
- Hacky high FPS (60 Hz) mode.
    - I've added a temporary hack to allow frame rates to exceed the 30Hz max of the original.
        - Many things are broken in this mode, including:
            - View bobbing.
            - Gravity being far too strong (related to this physics bug, see: https://www.youtube.com/watch?v=7RBycvyZf3I).
            - Not as smooth as it could be, occasional stuttering.
            - It also doesn't seem to work properly on MacOS.
    - Nonetheless you can enable to get an idea of how a high FPS PSX DOOM might feel.
        - Add the `-highfps` command line switch to enable it.
## Current limitations/bugs
- Sound is frequently out of sync with gameplay - not severely but sometimes it can be noticed. Probably due to emulation being either advanced too much or too little. Should eventually be fixed once sound engine advancement is tied to real-time.
- Occasionally the game will freeze when loading maps.
- Only the 'Greatest Hits' US version of PSX DOOM is supported, not Final DOOM or any other SKU.
- Multiplayer does not work and will freeze the game.
- The intro movie does not play yet.
- Probably other lots of other stuff not mentioned here...
