# PsyDoom MAPINFO documentation
To help modding, PsyDoom supports a 'MAPINFO' lump which allows information like level & episode names, music selection and so on to be defined. The purpose of this document is to list the definition types supported and fields within those definitions.
 
Some high level points to note:
- The lump must be defined in a main IWAD file (not a map WAD file). Normally you would use the extension IWAD `PSXDOOM_EXT.WAD` to do this. It can also be present in `PSXDOOM.WAD`.
- The syntax used is similar to the 'New ZDoom' format (see: https://zdoom.org/wiki/MAPINFO).
- All block and field names are case insensitive. Camel case is recommended however for readability.
- The MAPINFO lump is processed sequentially, from start to finish. This means that definitions like `ClearEpisodes` should be placed first before `Episode`.
- Unrecognized definition types or fields will be ignored.
- If the game already defines something (Map, Episode etc.) then defining it again in MAPINFO overwrites the existing definition. Furthermore, if some fields are omitted from the new definition, then those fields will be sourced from the original object. In this way you can use MAPINFO to partially tweak some existing game data, without having to redefine all of it. For example, you could alter the music track of an existing map without changing anything else.

## Episode
Defines an episode which can be selected on the main menu. Note: the game's episode list should be contiguous, with no missing episode numbers between the first and last. Example:
```
Episode 1 { 
    Name = "Hell To Pay!"
    StartMap = 1
}
Episode 2 { 
    Name = "Doomed..."
    StartMap = 10
}
```
Header fields:
- `1st`: The episode number which identifies the episode. Must be between 1 and 255.

Internal Fields:
- `StartMap`: Which map to load when starting a new game with this episode selected. Must be between 1 and 255.

## ClearEpisodes
When this definition is encountered it instructs the game to clear the current list of episodes. You can use it for example to remove unwanted episodes from Doom or Final Doom. Note: the definition doesn't have any other information associated with it, it's just a simple command/instruction:
```
ClearEpisodes {}
```
## Map
Defines various settings unique to each map. Full example (not all these fields need to be specified):
```
Map 3 "My Cool Map" {
    Music = 5
    Cluster = 1
    SkyPal = -1
    PlayCdMusic = 0
    ReverbMode = 6
    ReverbDepth = 0x27FF
    ReverbDelay = 0
    ReverbFeedback = 0
}
```
Header fields:
- `1st`: The map's number. Must be between 1 and 255.
- `2nd`: The name of the map, as shown on the automap and intermission screens.

Internal Fields:
- `Music`: Which sequencer music track to play for the map, or '0' if none. If 'PlayCdMusic' is enabled then this specifies a CD track instead. Must be between 0 and 1024.
- `Cluster`: Which cluster of maps the map belongs to. Determines when finale screens are shown; if the next map has a different cluster number then this will trigger the non-cast call finale.
- `SkyPal`: Can be used to override the palette used for the map's sky. Doom has 20 built-in palettes; Final Doom has 26. The engine can support up to 32 palettes in PLAYERPAL, so 6 additional user palettes can be defined. Specify `-1` (the default) to have the engine automatically determine the sky palette from the sky texture number. This field must be between -1 and 32. Built in palettes are as follows:
    ```
    MAINPAL               = 0       // Used for most sprites and textures in the game
    REDPALS (0-7)         = 1-8    // Pain palettes (red shift)
    BONUSPALS (0-3)       = 9-12   // Bonus pickup (gold shift) palettes
    RADIATIONPAL          = 13      // Radiation suit green shift
    INVULNERABILITYPAL    = 14      // PSX Doom: invulnerability effect
    FIRESKYPAL            = 15      // PSX Doom: fire sky palette
    UIPAL                 = 16      // PSX Doom: ui elements palette
    TITLEPAL              = 17      // PSX Doom: title screen palette
    IDCREDITS1PAL         = 18      // PSX Doom: ID credits screen palette
    WCREDITS1PAL          = 19      // PSX Doom: Williams credits screen palette
    UIPAL2                = 20      // PSX Final Doom: additional UI palette (used for plaques etc.)
    SKYPAL1               = 21      // PSX Final Doom: additional sky palette
    SKYPAL2               = 22      // PSX Final Doom: additional sky palette
    SKYPAL3               = 23      // PSX Final Doom: additional sky palette
    SKYPAL4               = 24      // PSX Final Doom: additional sky palette
    SKYPAL5               = 25      // PSX Final Doom: additional sky palette
    ```
- `PlayCdMusic`: If `1` or greater then `Music` is interpreted as a CD track to play instead of a music sequence. You can use this to play CD audio for the map instead of sequencer music.
- `ReverbMode`: Defines the type of reverb effect used for the map. Allowed reverb type numbers are:
    ```
    OFF        = 0      // No reverb
    ROOM       = 1
    STUDIO_A   = 2
    STUDIO_B   = 3
    STUDIO_C   = 4
    HALL       = 5
    SPACE      = 6
    ECHO       = 7      // Note: requires 'ReverbDelay' to be set and uses 'ReverbFeedback' instead of 'ReverbDepth'
    DELAY      = 8      // Note: requires 'ReverbDelay' to be set and uses 'ReverbFeedback' instead of 'ReverbDepth'
    PIPE       = 9
    ```
- `ReverbDepthL`: for most `ReverbMode` types (except `ECHO` and `DELAY`) this specifies the depth of the reverb effect (left channel). Allowed values are from -32768 to 32767.
- `ReverbDepthR`: for most `ReverbMode` types (except `ECHO` and `DELAY`) this specifies the depth of the reverb effect (right channel). Allowed values are from -32768 to 32767.
- `ReverbDepth`: This is an alias which sets both `ReverbDepthL` and `ReverbDepthR` at the same time. Most of the time you would probably want equal reverb on both channels.
- `ReverbDelay`: for the `ECHO` and `DELAY` reverb modes defines the delay for the effect. Allowed values are from -32768 to 32767.
- `ReverbFeedback`: for the `ECHO` and `DELAY` reverb modes defines effect strength. Allowed values are from -32768 to 32767.
## MusicTrack
This defines a sequencer (non-CDDA) music track. It can be used to add new music tracks, provided they exist in the game's WMD file. Example:
```
MusicTrack 1  { Sequence = 90  }    // The same as the built-in track '1'
MusicTrack 31 { Sequence = 136 }
MusicTrack 32 { Sequence = 137 }
```
Header fields:
- `1st`: The track number which identifies the track; this is what maps reference when specifying music. Must be between 1 and 1024.

Internal Fields:
- `Sequence`: The index of this music track's sequence in the Williams Module File (.WMD). This is the sequence number which will be played by the sequencer system. Must be between 0 and 16384.
