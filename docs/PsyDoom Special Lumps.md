# PsyDoom special lumps
Table of contents:
- [Overview](#Overview)
- [`MAPINFO`](#MAPINFO)
- [`PSYFANIM` and `PSYTANIM`](#PSYFANIM-and-PSYTANIM)
- [`PSYSWTEX`](#PSYSWTEX)
- [`PSYDECOR`](#PSYDECOR)
- [`SCRIPTS`](#SCRIPTS)

## Overview
This document details new lump types added for PsyDoom to support modding. The lump types allow for scripting, new animated textures, switches and custom decor sprites.

## `MAPINFO`
This lump should be added to `PSXDOOM_EXT.WAD` or `PSXDOOM.WAD`. It allows level and episode names and other game structure information to be defined. [The full documentation for MAPINFO can be found here: `PsyDoom MAPINFO.md`](PsyDoom%20MAPINFO.md)

## `PSYFANIM` and `PSYTANIM`
These lumps allow new floor and wall texture animations to be defined respectively. They should be added to `PSXDOOM_EXT.WAD` or `PSXDOOM.WAD`. The lumps are text format, where each individual line defines an animated floor or wall texture. Each line should contain the following:

```
BEGIN_TEX END_TEX TICMASK
```

Where:
- `BEGIN_TEX` is the start texture/flat of the animation.
- `END_TEX` is the end texture/flat of the animation.
- `TICMASK` is a bitwise mask applied to the current game tick number to determine if the animation advances. If the mask AND-ed with the current tick number equals `0` then the animation proceeds onto the next frame. Example values:
    - `0`: Change frames every game tick (every 1/15 seconds)
    - `1`: Change frames every 2nd game tick (every 2/15 seconds)
    - `3`: Change frames every 4th game tick (every 4/15 seconds)
    - `7`: Change frames every 8th game tick (every 8/15 seconds)

Example `PSYFANIM` lump defining new flat animations, using flat textures from imported from the 'OTEX' collection:

```
0ICYWA01 0ICYWA08 3
0LAVAC01 0LAVAC08 3
```

Example `PSYTANIM` lump defining new wall animations, using textures from imported from the 'OTEX' collection:

```
OFALLL01 OFALLL08 3
```

## `PSYSWTEX`
This lump allows new switch types to be defined. It should be added to `PSXDOOM_EXT.WAD` or `PSXDOOM.WAD`. The lump is text format, where each individual line defines a switch type. Each line should contain the following:

```
OFFTEX ONTEX
```

Where:
- `OFFTEX` is the texture to use for the switch 'on' state.
- `ONTEX` is the texture to use for the switch 'off' state.

Example `PSYSWTEX` lump defining new switches, using textures from imported from the 'OTEX' collection:

```
OSWTCHA4 OSWTCHA5
```

## `PSYDECOR`
This lump allows new decoration type objects/sprites to be defined. It should be added to `PSXDOOM_EXT.WAD` or `PSXDOOM.WAD`. The lump is text format, where each individual line defines a decor type. Each line should be in the following format:

```
DOOMEDNUM SPRITENAME FRAMES [RADIUS:X] [HEIGHT:X] [CEILING] [BLOCKING]
```

Where:
- `DOOMEDNUM` is the Doom editor number for the thing.
- `SPRITENAME` is a 4 character name for the sprite, e.g. 'CEYE'.
- `FRAMES` is a list of tic duration and frame letter pairs, in one unbroken string (no spaces). E.G: `2A3B4C` will play frame `A` for `2` tics, `B` for `3` tics and `C` for `4` tics.
- `RADIUS:X` optionally specifies the thing's radius as 'X'. Note: there must be no spaces between the colon and number.
- `HEIGHT:X` optionally specifies the thing's height as 'X'. Note: there must be no spaces between the colon and number.
- `CEILING` is an optional flag. If specified then the decor is attached to the ceiling.
- `BLOCKING` is an optional flag. If specified then the thing is treated as solid/blocking.

Examples:
```
200 BONE 1A
200 BONE 1A3B RADIUS:16 HEIGHT:64 CEILING BLOCKING
200 BONE 3B RADIUS:16 BLOCKING
```

## `SCRIPTS`
This lump should be added to each `MAPXX.WAD` file before any other lumps. It allows for custom Lua scripted actions to be defined for the level. [The full documentation for PsyDoom's Lua scripting can be found here: `PsyDoom Lua Scripting.md`](PsyDoom%20Lua%20Scripting.md)