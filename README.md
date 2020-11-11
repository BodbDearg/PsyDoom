# PsyDoom
This project backports PlayStation Doom and Final Doom to PC via reverse engineering. The code is derived directly from the original PlayStation machine code (see commit history for a timeline of its transformation) and now runs natively on modern systems, having been gradually converted from emulated MIPS instructions to structured C++ code over time. The [Avocado](https://github.com/JaCzekanski/Avocado) PlayStation emulator is used to handle the specifics of the PlayStation's GPU & SPU, helping the game look and sound as authentic as possible. The port is very accurate in terms of its gameplay and has been verified to reproduce exactly [4 playthroughs of PSX Doom & Final Doom (NTSC & PAL)](psxdoom_demos) given the original inputs stored in demo recordings. The game also has no external dependencies and just needs a valid .cue file of the original game disc to run.

A sister project, [PSXDOOM-RE](https://github.com/Erick194/PSXDOOM-RE), by [Erick Vásquez García (Erick194)](https://github.com/Erick194) also completely recreates the Doom source code for the actual PlayStation hardware and 'PsyQ' SDK. The reverse engineering work for that project was used to help accelerate the transition to native C++ code for this project and to cross verify the reverse engineering work in both projects.

As of right now the game mostly runs correctly, with a few very minor issues. Here is a brief video demonstration showing the project in action:

[![Alt text](https://img.youtube.com/vi/o7t7w1YjjSw/0.jpg)](https://www.youtube.com/watch?v=o7t7w1YjjSw)

The original goal of this project was to have a complete replacement for the original PlayStation Doom .EXE for modern systems. Now that this objective has been achieved, the focus turns to adding quality of life improvements, some modding features and additional polish.

## Running
- Download the latest release build here: https://github.com/BodbDearg/PsyDoom/releases
- Make sure to have the latest Visual C redistributable installed:
  - https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads
- The following items must be present in the working directory of the application:
  - **DOOM.CUE**: A .cue file pointing to all of the tracks and binary data for the game disc. 
    - Any original edition of the game is acceptable: NTSC-U, NTSC-J, PAL, NTSC-U 'Greatest Hits' etc.
    - Both Doom and Final Doom can be used.
- The settings for the game can be changed by editing the .ini files found in the following folders:
  - Windows: `%APPDATA%\com.codelobster\PsyDoom`
  - MacOS: `~/Library/Application Support/com.codelobster/PsyDoom`
- Keyboard controls currently default to the following:
  - Up/W and Down/S : Move Forward and Move Backward
  - A and D : Strafe Left and Right
  - Left and Right : Turn Left and Right
  - Space : Use
  - Ctrl/F : Fire
  - Shift : Run
  - PgUp/E PgDown/Q : Next Weapon and Previous Weapon
  - P/Pause/Return : Pause
  - Tab : Automap
  - Alt : Automap Manual Move
  - -/+ : Automap Zoom Out or In
  - Numbers 1-8 : Weapon switch
- Mouse controls are currently defaulted to:
  - Left mouse : Fire
  - Right mouse : Use
  - Wheel : Next / Previous Weapon
- Xbox One/360 Controller bindings are currently defaulted to:
  - Right Stick : Turn
  - Left Stick : Move
  - Left Trigger : Run
  - Right Trigger / Y : Fire
  - B : Use
  - A : Automap Pan
  - Shoulder Buttons : Next or Previous Weapon
  - Start : Pause
  - Back : Toggle Automap
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
- File override modding system.
    - You can override any game files by supplying the game with a directory containing those overrides.
    - Specify the directory using the `-datadir <MY_DIRECTORY_PATH>` command line argument.
    - Put files in this folder (note: not in any child folders!) that you wish to override, e.g 'MAP01.WAD'.
    - If the game goes to load a file such as 'MAP01.WAD' and it is present in the overrides dir, then the on-disk version will be used instead.
- Other miscellaneous command line arguments
    - The .cue file used can be manually specified on launch via `-cue <CUE_FILE_PATH>`.
    - For a 'no monsters' cheat similar to PC Doom use the `-nomonsters` switch.
    - To play a demo lump file and exit use `-playdemo <DEMO_LUMP_FILE_PATH>`.
    - To save the results of demo playback to a .json file use `-saveresult <RESULT_FILE_PATH>`.
    - To verify that the result of demo playback matches a result .json file use `-checkresult <RESULT_FILE_PATH>`. If the result matches the expected result, the return code from the executable will be '0'. On an unexpected result, a non-zero return code is returned.
    - To run the game in headless mode (for demo playback) use `-headless`

## Current limitations/bugs
- The intro movie does not play yet.

## How to build
- Requires a recent CMake to generate the project (3.15 or higher)
- Builds with Visual Studio 2019 (Windows 64-bit) and also Xcode 11 on MacOS. Other IDEs and toolchains may work but are untested.
