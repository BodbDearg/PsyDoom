# PsyDoom
This project backports PlayStation Doom and Final Doom to PC via reverse engineering. The code is derived directly from the original PlayStation machine code (see commit history for a timeline of its transformation) and now runs natively on modern systems, having been gradually converted from emulated MIPS instructions to structured C++ code over time. 

The specifics of the PlayStation's GPU (for the original renderer) and the SPU are internally emulated, helping the game look and sound as authentic as possible. A new Vulkan renderer supporting widescreen & high resolutions is also available for systems that support it (enabled by default), allowing PlayStation Doom to be experienced in much higher definition.

The port is very accurate in terms of its gameplay and has been verified to reproduce exactly [4 playthroughs of PSX Doom & Final Doom (NTSC & PAL)](extras/psxdoom_demos) given the original player inputs stored in demo recordings. The game also has no external dependencies and just needs a valid .cue file of the original game disc to run.

A sister project, [PSXDOOM-RE](https://github.com/Erick194/PSXDOOM-RE), by [Erick Vásquez García (Erick194)](https://github.com/Erick194) also completely recreates the Doom source code for the actual PlayStation hardware and 'PsyQ' SDK. The reverse engineering work for that project was used to help accelerate the transition to native C++ code for this project and to cross verify the reverse engineering work in both projects.

Here is a recent video showing the port in action:

[![Alt text](https://img.youtube.com/vi/0miyRHptfeA/0.jpg)](https://youtu.be/0miyRHptfeA)

The original goal of this project was to have a complete replacement for the original PlayStation Doom .EXE for modern systems. Now that this objective has been achieved, the focus turns to adding quality of life improvements, some modding features and additional polish.

## README contents:
- [Running the game](#Running-the-game)
- [Controls](#Controls)
- [Running and creating user mods](#Running-and-creating-user-mods)
- [Multiplayer/link-cable emulation](#Multiplayerlink-cable-emulation)
- [Command line arguments](#Command-line-arguments)
- [Current limitations/bugs](#Current-limitationsbugs)
- [How to build](#How-to-build)

## Running the game
- Download the latest release build from here: https://github.com/BodbDearg/PsyDoom/releases
- Presently only Windows 64-bit and MacOS (ARM/Intel 64-bit) are supported. Linux is not currently supported but is planned for a future release.
  - The Vulkan renderer also requires a Vulkan 1.1 capable GPU. On MacOS a 'Metal' capable GPU is required.
- On Windows, make sure to have the latest Visual C redistributable installed:
  - https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads
- On MacOS you must manually allow the .app through Gatekeeper in order to run the game, as the code is not signed.
  - You must also enter the full path to DOOM.CUE in your `game_cfg.ini` file; PsyDoom cannot detect the .cue file even if it is placed in the same directory as the .app file due to OS security measures ('translocation').
- The following items must be present in the working directory of the application:
  - **DOOM.CUE**: A .cue file pointing to all the tracks and binary data for one of the original game discs.
    - Any edition of the original game is acceptable: NTSC-U, NTSC-J, PAL, NTSC-U 'Greatest Hits' etc.
    - Both Doom and Final Doom can be used.
- The settings for the game can be changed by editing the .ini files found in the following folders:
  - Windows: `%APPDATA%\com.codelobster\PsyDoom`
  - MacOS: `~/Library/Application Support/com.codelobster/PsyDoom`
- For the best audio quality, set your audio device's sample rate to 44.1 kHz (44,100 Hz or 'CD Quality').
  - Sometimes using different sample rates like 48 kHz can result in strange noise/artifacts when the audio stream is resampled to a different rate by the host system.
  - 44.1 kHz is the sample rate originally used by the PS1 SPU and the native sample rate of PsyDoom.

## Controls
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
  - \` : Toggle between the Vulkan and Classic (original) renderers, hardware permitting.
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

## Running and creating user mods
- User mods are supplied in the form of a directory.
- Specify where the mod directory is located using the `-datadir <MY_DIRECTORY_PATH>` command line argument.
- For details on creating new content using PsyDoom's modding system, see the following documents:
  - [PsyDoom Modding Overview.md](docs/PsyDoom%20Modding%20Overview.md)
  - [PsyDoom Special Lumps.md](docs/PsyDoom%20Special%20Lumps.md)
  - [PsyDoom MAPINFO.md](docs/PsyDoom%20MAPINFO.md)
  - [PsyDoom Lua Scripting.md](docs/PsyDoom%20Lua%20Scripting.md)

## Multiplayer/link-cable emulation
- PsyDoom now supports an emulation of the original 'Link Cable' multiplayer functionality, over regular TCP.
- This requires a VERY low latency network connection to be playable as the game's network protocol is synchronous and not lag tolerant, Ethernet or very fast Wifi is HIGHLY recommended.
- Player 1 is the 'server' and listens for connections from Player 2, the 'client'.
- To specify the machine as a server, add the following command line switch (listen port is optional, defaults to `666`):
    - `-server [LISTEN_PORT]`
- To specify the current machine as a client, add the `-client` switch and specify the server host name afterwards:
    - `-client [SERVER_HOST_NAME_AND_PORT]` 
- If you need to specify a server port other than the default `666`, use the following format:
    - `-client 192.168.0.2:12345`

## Command line arguments
- Specify a user mod directory via the `-datadir <MY_DIRECTORY_PATH>` argument.
- The .cue file used can be manually specified on launch via `-cue <CUE_FILE_PATH>`.
- See [Multiplayer/link-cable emulation](#Multiplayerlink-cable-emulation) for multiplayer related arguments.
- To add extra IWADs to the game, use the `-file <WAD_FILE_PATH>` argument.
    - See [Extension IWADS](docs/PsyDoom%20Modding%20Overview.md#Extension-IWADS) in the [PsyDoom Modding Overview](docs/PsyDoom%20Modding%20Overview.md) for more details on this.
- For a 'no monsters' cheat similar to PC Doom use the `-nomonsters` switch.
- To force pistol starts on all levels, use the `-pistolstart` switch. This setting also affects password generation and multiplayer.
- To enable the 'turbo mode' cheat, use the `-turbo` switch. This setting allows the player to move and fire 2x as fast. Doors and platforms also move 2x as fast. Monsters are unaffected.
- To play a demo lump file and exit use `-playdemo <DEMO_LUMP_FILE_PATH>`.
  - Note that this also causes intro screens to be skipped.
- To save the results of demo playback to a .json file use `-saveresult <RESULT_FILE_PATH>`.
- To verify that the result of demo playback matches a result .json file use `-checkresult <RESULT_FILE_PATH>`. If the result matches the expected result, the return code from the executable will be '0'. On an unexpected result, a non-zero return code is returned.
- To record demos for each map played, use the `-record` switch. Notes on this:
  - Pausing the game ends demo recording. In multiplayer any player pausing will end recording.
  - Demos will only be recorded when playing from the start of the map, not when starting from a save game.
  - Demos will be named `DEMO_MAP??.LMP` after the current map number and output to the user settings and data directory.
  - To find the user settings and data directory, see: [Running The Game](#Running-the-game).
- To run the game in headless mode (for demo playback only) use `-headless`

## Current limitations/bugs
- The intro movie does not play yet. This functionality will be implemented in a future build.

## How to build
- Requires a recent CMake to generate the IDE specific project files (3.15 or higher)
- Builds with Visual Studio 2019 (Windows 64-bit) and also Xcode 11 on MacOS. Other IDEs and toolchains may work but are untested.
- Note: Only Windows and MacOS are valid platforms to build on, Linux is not yet supported.
- On MacOS you must download and install the Vulkan SDK in order to be able to build with the Vulkan renderer enabled.
