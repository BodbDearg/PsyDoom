# PsyDoom
*(Note: formerly known as 'StationDoom')*

This project backports PlayStation Doom to PC using reverse engineering. The code is derived directly from the original PlayStation machine code (see commit history for a timeline of its transformation) and now runs natively on modern systems, having been gradually converted from emulated MIPS instructions to structured C++ code over time. The [Avocado](https://github.com/JaCzekanski/Avocado) PlayStation emulator is used to handle the specifics of some PSX hardware and replicate the GPU & SPU of the PlayStation, helping the game feel authentic as possible. As of right now the game runs 100% natively on PC but still requires the original .EXE file for some global variable values; this limitation will be removed soon.

A sister project, [PSXDOOM-RE](https://github.com/Erick194/PSXDOOM-RE), by [Erick Vásquez García (Erick194)](https://github.com/Erick194) also completely recreates the Doom source code for the actual PlayStation hardware and 'PsyQ' SDK. The reverse engineering work for that project was used to help accelerate the transition to native C++ code for this project and to cross verify the reverse engineering work in both projects.

Will add more details here later and eventually 'official' binary builds once it is stable enough for general release. If you want to try/experiment with this for now, you will need to build from source or use one of the occasional binaries that I will put up - see details below.

As of right now the game mostly runs correctly, with a few minor issues. Here is a brief video demonstration showing the project in action:

[![Alt text](https://img.youtube.com/vi/o7t7w1YjjSw/0.jpg)](https://www.youtube.com/watch?v=o7t7w1YjjSw)

The eventual goal of this project is to have a complete replacement for the original PlayStation Doom .EXE for modern systems, and allow for modern conveniences like keyboard & mouse support etc.

Longer term goals include support for Final Doom, proper modding support (I added some basic support so far!) and PSX DOOM engine limit removal. Once all those things have been done this project can also be forked to provide an 'enhanced' version of game with support for higher resolution, fixing of graphical glitches, support for widescreen and so on.

## How to build
- Requires a recent CMake to generate the project (3.15 or higher)
- Builds with Visual Studio 2017/2019 (Windows 64-bit) and also Xcode 11 on MacOS. Other platforms & IDEs not yet supported/tested.

## Running
- Make sure to have the latest Visual C redistributable installed:
  - https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads
- The following items must be present in the working directory of the application:
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
- Multiplayer/link-cable emulation
    - PsyDoom now supports an emulation of the original 'Link Cable' multiplayer functionality, over regular TCP.
    - This requires a VERY low latency network connection to be playable as the game's network protocol is synchronous and not lag tolerant, Ethernet or very fast Wifi is HIGHLY recommended.
    - Player 1 is the 'server' and listens for connections from Player 2, the 'client'.
    - To specify the machine as a server, add the following command line switch (listen port is optional, defaults to `666`):
        - `-server [LISTEN_PORT]`
    - To specify the current machine as a client, add the `-client` switch and specify the server host name afterwards:
        - `-client [SERVER_HOST_NAME_AND_PORT]` 
    - If you need to specify a server port other than the default `666`, use the following format:
        - `-client 192.168.0.2:12345`
- Hacky/temp 'mods' system.
    - You can override any game files by supplying the game with a directory containing those overrides.
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
- Some occasional sound issues, sound is mostly OK at this point though.
- Only the 'Greatest Hits' US version of PSX DOOM is supported, not Final DOOM or any other SKU.
- The intro movie does not play yet.
