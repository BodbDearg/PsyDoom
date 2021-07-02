# PsyDoom MAPINFO documentation
To help modding, PsyDoom supports a 'MAPINFO' lump which allows information like level & episode names, music selection and so on to be defined. This MAPINFO uses similar syntax to the 'New ZDoom' format (see: https://zdoom.org/wiki/MAPINFO). The purpose of this document is to list the definition types supported and fields within those definitions.

## Episode
Defines an episode which can be selected on the main menu. Note: the game's episode list should be contiguous, with no missing episode numbers between the first and last. Example:
```
Episode 1 { 
    Name = "Hell To Pay!"
    StartMap = 1
}
Episode 2 { 
    Name = "Doomed"
    StartMap = 10
}
```
Header fields:
- `1st`: The episode number which identifies the episode. Must be between 1 and 255.

Internal Fields:
- `StartMap`: Which map to load when starting a new game with this episode selected. Must be between 1 and 255.

## MusicTrack
This data structure defines a sequencer (non-CDDA) music track. It can be used to add new music tracks, provided they exist in the game's WMD file. Example:
```
MusicTrack 1  { Sequence = 90  }    // The same as the built-in track '1', will modify the existing definition if different
MusicTrack 31 { Sequence = 136 }
MusicTrack 32 { Sequence = 137 }
```
Header fields:
- `1st`: The track number which identifies the track; this is what maps reference when specifying music. Must be between 1 and 1024.

Internal Fields:
- `Sequence`: The index of this music track's sequence in the Williams Module File (.WMD). This is the sequence number which will be played by the sequencer system. Must be between 0 and 16384.
