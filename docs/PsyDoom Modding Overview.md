# PsyDoom modding overview
Table of contents:
- [The basics](#The-basics)
- [Extension IWADS](#Extension-IWADS)
- [Level Editor](#Level-Editor)
- [`ALLMAPS.LCD`](#ALLMAPSLCD)
- [PsyDoom limit removing and extended features](#PsyDoom-limit-removing-and-extended-features)

In-depth specifications:
- [Special Lumps](PsyDoom%20Special%20Lumps.md)
- [`MAPINFO`](PsyDoom%20MAPINFO.md)
- [Lua Scripting](PsyDoom%20Lua%20Scripting.md)

## The basics
- Unlike the PC engine, user mods for PsyDoom are supplied in the form of a directory instead of a single WAD file.
- The user specifies which mod directory to use via the `-datadir <MY_DIRECTORY_PATH>` command line argument.
- Files found in this folder will override existing files on disc.
    - For example, if the game goes to load `MAP01.WAD` and it is present in the mod directory, then the mod directory version will be used instead.
    - Note: only the root of this mod directory will be searched for files. Directory structure is also ignored during override matching; only filenames are compared.
- `MAPXX.WAD` files (map WADS) can only be used to contain data and [scripts](PsyDoom%20Lua%20Scripting.md) for a single level; all other contents of these WAD files will be ignored. To add graphics or other resources to the game, they must be added to the main IWAD `PSXDOOM.WAD` or preferably defined in an [Extension IWAD (see below)](#Extension-IWADS).
    - Conversely, new maps cannot be added via the IWADs and any map data contained within them will be ignored.

## Extension IWADS
PsyDoom allows the use of extension IWADS in the mod directory to add (or override) the following resources:
- Any kind of resource found in `PSXDOOM.WAD`, such as texture and sprite lumps.
- [`MAPINFO`](PsyDoom%20MAPINFO.md) for naming levels and defining game/episode structure.
- [`PSYFANIM` and `PSYTANIM`](PsyDoom%20Special%20Lumps.md#PSYFANIM-and-PSYTANIM) lumps for adding new floor and wall texture animations respectively.
- A [`PSYSWTEX`](PsyDoom%20Special%20Lumps.md#PSYSWTEX) lump for defining new switch types.
- A [`PSYDECOR`](PsyDoom%20Special%20Lumps.md#PSYDECOR) lump for defining new decoration sprites.

PsyDoom searches for 2 specific extension IWAD files in the mod directory, and will use them when present:
- `PSXDOOM_EXT.WAD` this is intended to be an extension to the main IWAD containing resources specific to the map set. For example, new textures or sprites used by the levels.
- `PSX_MISSING_ENEMIES.WAD` this is another extension IWAD, intended to contain sprites for missing enemies such as the Arch Vile. This WAD is intended to be somewhat re-usable between different mods, hence the separation. This extension IWAD also has lower precedence than `PSXDOOM_EXT.WAD`, meaning files in `PSXDOOM_EXT.WAD` will override conflicting files in `PSX_MISSING_ENEMIES.WAD`.

Additional IWADs (not part of the mod) can also be specified by the user 1 at a time via the `-file <WAD_FILE_PATH>` command line argument. These take precedence over the regular game IWAD and all extension IWADs. Later arguments have most precedence.
- This could be used for instance to add a custom status bar while playing a new map set.

## Level Editor
To make levels for PsyDoom or the original PSX Doom, a special fork of GZDoom Builder must be used called "GZDoom Builder Custom By GEC V3". Download links to the latest version of this tool and more instructions on it's use can be found in the [Doomworld thread for the "GEC Master Edition PSX Doom"](https://www.doomworld.com/forum/topic/101161-gec-master-edition-psx-doom-for-the-playstation-1102019-beta-3-release-now-are-you-ready-for-more-action/).

At the time of writing the released version (V3) does not officially support PsyDoom, though maps made with this tool can still be run in PsyDoom. For the very latest version which supports PsyDoom extensions, and wall heights greater than 256 units, you must build and run the tool from source. The latest source code can be found here:
https://github.com/Erick194/DoomBuilderPSX

Some additional points to note when using this tool for PsyDoom mapping specifically:
- PsyDoom does not require `MAPSPRXX.IMG` and `MAPTEXXX.IMG` files to be generated via the scripts supplied with this custom GZDoom Builder. PsyDoom ignores these files and instead loads all graphical resources from the main IWAD files. This means you can simply save a WAD file in GZDoom builder immediately test it without having to run any scripts.
- Similarly you do not need generate a `MAPXX.LCD` file containing monster sounds for the level if [`ALLMAPS.LCD` is used instead (see below)](#ALLMAPSLCD).

## `ALLMAPS.LCD`
In order to simplify editing maps, PsyDoom will search for a file called `ALLMAPS.LCD` in the mod directory. This file should contain all possible monster sounds for any level in the game. If the game sees this file is present, it will not attempt to load any of the `MAPXX.LCD` files and will instead use this sound archive. Loading all enemy sounds in the game is now possible because PsyDoom has a greatly enlargened sound RAM capacity.

The PsyDoom GIT repository supplies a version of this file which can be redistributed with user levels. It is found in the following folder:
[extras/psydoom_mapping](../extras/psydoom_mapping)

If you use this `ALLMAPS.LCD` file then you can avoid generating `MAPXX.LCD` files via the custom GZDoom Builder scripts, thus simplifying the mapping workflow greatly.

## PsyDoom limit removing and extended features
PsyDoom has the following new or limit removing features versus the original PlayStation engine. This means more complex and customized creations are now possible:
- Extended VRAM for sprites and textures: instead of `1 MiB` PsyDoom can now have up to `128 MiB` of VRAM available (the default).
- Extended SRAM for sounds and music: increased from `512 KiB` to a default of `16 MiB` and user extendable beyond that.
- Extended main RAM for general game data. Increased from `~1.3 MiB` to `64 MiB` by default, and user extendable beyond that.
- Added support for any power of two texture size from `2x2` up to `1024x512`.
- The number of PSX format palettes that can be stored in `PLAYPAL` has been raised from `26` to `32`. Final Doom uses `26` palettes so this allows an extra `6` user palettes.
- Increased number of SPU voices (`64` instead of `24`).
- Floating point mixing for the SPU; eliminates clipping artifacts if sound gets too loud.
- A [Lua scripting engine](PsyDoom%20Lua%20Scripting.md) for more complex special actions.
- Generic 'marker' thing types intended to be used with scripting.
- Doom 64 style dual colored lighting.
- Classic renderer: increased the maximum number of draw sprites per sector (`8192` instead of `64`).
- Classic renderer: added support for walls higher than `256` units.
- Removed all limits on:
    - The maximum map WAD lump size (was `64 KiB`).
    - The number of scrollable lines, moving floors, active ceilings and crushers.
    - The number of line specials that can be triggered in one move/tick.
    - The number of switches that can be switched at once.
    - Classic renderer: the number of subsectors that can be drawn.
    - Classic renderer: the amount of geometry that can be clipped.
    - The size of the sequencer module file (`DOOMSND.WMD` file)
- Fixes to the classic renderer to support very tall walls.
- Added support for floor skies.
- Added linedef flags to control when sky walls render and to control occlusion culling for fake 3D effects.
- Added support for invisible 'ghost' platforms that render lower than what they are.
- Added support for missing enemies (Arch Vile, Icon Of Sin etc.) and a couple of linedef and sector types only found in PC Doom II.
- 'Texture cache overflow' fatal error is now just a warning (if VRAM limits have been exceeded).
