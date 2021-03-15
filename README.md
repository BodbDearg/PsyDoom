# PsyDoom
This project backports PlayStation Doom and Final Doom to PC via reverse engineering. The code is derived directly from the original PlayStation machine code (see commit history for a timeline of its transformation) and now runs natively on modern systems, having been gradually converted from emulated MIPS instructions to structured C++ code over time. The specifics of the PlayStation's GPU and the SPU are internally emulated, helping the game look and sound as authentic as possible. The port is very accurate in terms of its gameplay and has been verified to reproduce exactly [4 playthroughs of PSX Doom & Final Doom (NTSC & PAL)](extras/psxdoom_demos) given the original per-frame inputs stored in demo recordings. The game also has no external dependencies and just needs a valid .cue file of the original game disc to run.

A sister project, [PSXDOOM-RE](https://github.com/Erick194/PSXDOOM-RE), by [Erick Vásquez García (Erick194)](https://github.com/Erick194) also completely recreates the Doom source code for the actual PlayStation hardware and 'PsyQ' SDK. The reverse engineering work for that project was used to help accelerate the transition to native C++ code for this project and to cross verify the reverse engineering work in both projects.

As of right now the game mostly runs correctly, with a few very minor issues. Here is a recent video showing the project in action:

[![Alt text](https://img.youtube.com/vi/ohS8tYPNr0M/0.jpg)](https://www.youtube.com/watch?v=ohS8tYPNr0M)

The original goal of this project was to have a complete replacement for the original PlayStation Doom .EXE for modern systems. Now that this objective has been achieved, the focus turns to adding quality of life improvements, some modding features and additional polish.

## Running
- Presently only Windows 64-bit and MacOS (Intel 64-bit) are supported. Linux is not currently supported but is planned for a future release.
- Download the latest release build from here: https://github.com/BodbDearg/PsyDoom/releases
- On Windows, make sure to have the latest Visual C redistributable installed:
  - https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads
- The following items must be present in the working directory of the application:
  - **DOOM.CUE**: A .cue file pointing to all of the tracks and binary data for the game disc. 
    - Any original edition of the game is acceptable: NTSC-U, NTSC-J, PAL, NTSC-U 'Greatest Hits' etc.
    - Both Doom and Final Doom can be used.
- The settings for the game can be changed by editing the .ini files found in the following folders:
  - Windows: `%APPDATA%\com.codelobster\PsyDoom`
  - MacOS: `~/Library/Application Support/com.codelobster/PsyDoom`
- For the best audio quality, set your audio device's sample rate to 44.1 kHz (44,100 Hz or 'CD Quality').
  - Sometimes using different sample rates like 48 kHz can result in strange noise/artifacts when the audio stream is resampled to a different rate by the host system.
  - 44.1 kHz is the sample rate originally used by the PS1 SPU and the native sample rate of PsyDoom.
- Keyboard controls currently default to the following:
  - Up/W and Down/S : Move Forward and Move Backward
  - A and D : Strafe Left and Right
  - Left and Right : Turn Left and Right
  - Space : Use
  - Ctrl/F : Fire
  - Shift : Run
  - Caps Lock : Toggle autorun
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
- In addition to entering the original PSX cheat buttons with a controller, the following cheat key sequences are supported:
  - iddqd : God mode
  - idclip : No-clip mode
  - idclev : Open level warp
  - idkfa : Give weapons, keys, ammo and armor
  - iddt : Show all map lines
  - idmt : Show all map things
  - idray : X-ray vision
  - idram : Open the VRAM viewer
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
    - To force pistol starts on all levels, use the `-pistolstart` switch. This setting also affects password generation and multiplayer.
    - To play a demo lump file and exit use `-playdemo <DEMO_LUMP_FILE_PATH>`.
    - To save the results of demo playback to a .json file use `-saveresult <RESULT_FILE_PATH>`.
    - To verify that the result of demo playback matches a result .json file use `-checkresult <RESULT_FILE_PATH>`. If the result matches the expected result, the return code from the executable will be '0'. On an unexpected result, a non-zero return code is returned.
    - To run the game in headless mode (for demo playback) use `-headless`

## Current limitations/bugs
- The intro movie does not play yet. This functionality will be implemented in a future build.

## How to build
- Requires a recent CMake to generate the IDE specific project files (3.15 or higher)
- Builds with Visual Studio 2019 (Windows 64-bit) and also Xcode 11 on MacOS. Other IDEs and toolchains may work but are untested.
- Note: Only Windows and MacOS are valid platforms to build on, Linux is not yet supported.
