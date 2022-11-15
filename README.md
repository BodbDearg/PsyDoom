# PsyDoom
This project backports PlayStation Doom and Final Doom to PC via reverse engineering. The code is derived directly from the original PlayStation machine code (see commit history for a timeline of its transformation) and now runs natively on modern systems, having been gradually converted from emulated MIPS instructions to structured C++ code over time. 

The specifics of the PlayStation's GPU (for the original renderer) and the SPU are internally emulated, helping the game look and sound as authentic as possible. A new Vulkan renderer supporting widescreen & high resolutions is also available and enabled by default for systems that support it, allowing PlayStation Doom to be experienced in much higher definition. Modding support, extended engine limits and capabilities are also built-in.

The port is very accurate in terms of its gameplay and has been verified to reproduce exactly [4 playthroughs of PSX Doom & Final Doom (NTSC & PAL)](extras/psxdoom_demos) given the original player inputs stored in demo recordings. The game also has no external dependencies and just needs a valid .cue file of the original game disc to run.

A sister project, [PSXDOOM-RE](https://github.com/Erick194/PSXDOOM-RE) by [Erick Vásquez García (Erick194)](https://github.com/Erick194), also completely recreates the Doom source code for the actual PlayStation hardware and 'PsyQ' SDK. The reverse engineering work for that project was used to help accelerate the transition to native C++ code for this project and to cross verify the reverse engineering work in both projects.

Here is a video showing the port in action:

[![Alt text](https://img.youtube.com/vi/0miyRHptfeA/0.jpg)](https://youtu.be/0miyRHptfeA)

## README contents:
- [Requirements](#Requirements)
- [Running the game](#Running-the-game)
- [Running and creating user mods](#Running-and-creating-user-mods)
- [Multiplayer/link-cable emulation](#Multiplayerlink-cable-emulation)
- [Command line arguments](#Command-line-arguments)
- [How to build](#How-to-build)

## Other documents
- [License](LICENSE)
- [Contributors](CONTRIBUTORS.md)

## Requirements
- Supported platforms are Windows, macOS or Linux.
    - Supported versions of Linux are Debian 'Buster' based distros or later. Other variants of Linux *may* work but have not been tested.
- The Vulkan renderer requires a Vulkan 1.0 capable GPU or it will be unavailable. On macOS a 'Metal' capable GPU is required.
- On Windows make sure to have the latest Visual C redistributable installed:
    - https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads
- On MacOS you must manually allow the .app through Gatekeeper in order to run the game, as the code is not signed.
- On Linux you must have 'PulseAudio' installed to hear sound.

## Running the game
- Download the latest release build from here: https://github.com/BodbDearg/PsyDoom/releases
- A valid game disc in the form of a '.cue' file must be provided before launching. The '.cue' file describes the contents of the game disc and where all the data for the disc is located. Supported game discs include:
    - Doom (all regions & editions)
    - Final Doom (all regions & editions)
    - Doom: single level demo (standalone and magazine demo disc)
    - [GEC Master Edition Beta 3](https://www.doomworld.com/forum/topic/101161-gec-master-edition-psx-doom-for-the-playstation-1102019-beta-3-release-now-are-you-ready-for-more-action)
- Note: for the best audio quality, set your audio device's sample rate to 44.1 kHz (44,100 Hz or 'CD Quality').
    - Sometimes using different sample rates like 48 kHz can result in strange noise/artifacts when the audio stream is resampled to a different rate by the host system.
    - 44.1 kHz is the sample rate originally used by the PS1 SPU and the native sample rate of PsyDoom.

## Running and creating user mods
- User mods are supplied in the form of a directory.
- Specify where the mod directory is located using launcher or via the `-datadir <MY_DIRECTORY_PATH>` command line argument.
- For details on creating new content using PsyDoom's modding system, see the following documents:
    - [PsyDoom Modding Overview.md](docs/PsyDoom%20Modding%20Overview.md)
    - [PsyDoom Special Lumps.md](docs/PsyDoom%20Special%20Lumps.md)
    - [PsyDoom MAPINFO.md](docs/PsyDoom%20MAPINFO.md)
    - [PsyDoom Lua Scripting.md](docs/PsyDoom%20Lua%20Scripting.md)

## Multiplayer/link-cable emulation
- PsyDoom now supports an emulation of the original 'Link Cable' multiplayer functionality, over regular TCP.
- This requires a VERY low latency network connection to be playable as the game's network protocol is synchronous and not lag tolerant.
    - Ethernet or very fast Wifi is HIGHLY recommended!
- Player 1 is the 'server' and listens for connections from Player 2, the 'client'.
- If unspecified, the default port used for client/server communication is `666` on Windows and macOS and `1666` on Linux.
- To run a multiplayer game, start a co-op or deathmatch game using one machine as a server and wait for a connection from the client. The client connects by trying to start any type of multiplayer game from the main menu.
- Note that both the server and client must be running a compatible versions of the game.
- The server also decides the game rules and overrides any gameplay settings the client may have.

## Command line arguments
- Specify a user mod directory via the `-datadir <MY_DIRECTORY_PATH>` argument.
- The .cue file used can be manually specified on launch via `-cue <CUE_FILE_PATH>`.
- To add extra IWADs to the game, use the `-file <WAD_FILE_PATH>` argument.
    - See [Extension IWADS](docs/PsyDoom%20Modding%20Overview.md#Extension-IWADS) in the [PsyDoom Modding Overview](docs/PsyDoom%20Modding%20Overview.md) for more details on this.
- For a 'no monsters' cheat similar to PC Doom use the `-nomonsters` switch.
- To trigger (otherwise broken) boss related special actions at the start of maps when the 'no monsters' cheat is active, use the '-nmbossfixup' switch. This for example will open the doors in 'Phobos Anomaly' which require all Barons Of Hell to be killed.
- To force pistol starts on all levels, use the `-pistolstart` switch. This setting also affects password generation and multiplayer.
- To enable the 'turbo mode' cheat, use the `-turbo` switch. This setting allows the player to move and fire 2x as fast. Doors and platforms also move 2x as fast. Monsters are unaffected.
- To warp directly to a specified map on startup use `-warp <MAP_NUMBER>`.
- To specify the skill level (0-4) for warping to a map on startup map use `-skill <SKILL_NUMBER>`. Skill level '0' is 'I am a Wimp' and level '4' is 'Nightmare!'.
- To play a demo lump file and exit use `-playdemo <DEMO_LUMP_FILE_PATH>`.
    - Note that this also causes intro screens to be skipped.
- To save the results of demo playback to a .json file use `-saveresult <RESULT_FILE_PATH>`.
- To verify that the result of demo playback matches a result .json file use `-checkresult <RESULT_FILE_PATH>`. If the result matches the expected result, the return code from the executable will be '0'. On an unexpected result, a non-zero return code is returned.
- To record demos for each map played, use the `-record` switch. Notes on this:
    - Pausing the game ends demo recording. In multiplayer any player pausing will end recording.
    - Demos will only be recorded when playing from the start of the map, not when starting from a save game.
    - Demos will be named `DEMO_MAP??.LMP` after the current map number and output to the user settings and data directory.
    - To find the user settings and data directory, see: [Running The Game](#Running-the-game).
- To run the game in headless mode (for demo playback only) use `-headless`.
- Multiplayer related arguments:
    - To specify the current machine as a server and optionally use a port other than the default:
        - `-server [LISTEN_PORT]`
    - To specify the current machine as a client and connect to a specific host name or IP address:
        - `-client [SERVER_HOST_NAME_AND_PORT]` 
    - As a client if you need to specify a server port other than the default use the following format:
        - `-client 192.168.0.2:12345`
- To skip showing the launcher on startup specify `-nolauncher` or any other command line argument.

## How to build
- Requires CMake 3.13.4 or higher to generate the platform specific project files and/or build scripts.
- Builds with Visual Studio 2019 (Windows 64-bit) and also Xcode 11 on MacOS. On Linux, GCC 8 was used to compile. Other IDEs and toolchains may work but are untested.
- On MacOS you must download and install the Vulkan SDK in order to be able to build with the Vulkan renderer enabled.
