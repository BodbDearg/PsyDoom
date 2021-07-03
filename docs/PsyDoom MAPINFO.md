# PsyDoom MAPINFO documentation
To help modding, PsyDoom supports a 'MAPINFO' lump which allows information like level & episode names, music selection and so on to be defined. The purpose of this document is to list the definition types supported and fields within those definitions.
 
 Some high level points to note:
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
