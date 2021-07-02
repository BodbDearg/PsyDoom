# PsyDoom MAPINFO documentation
To help modding, PsyDoom supports a 'MAPINFO' lump which allows information like level & episode names, music selection and so on to be defined. This MAPINFO uses similar syntax to the 'New ZDoom' format (see: https://zdoom.org/wiki/MAPINFO). The purpose of this document is to list the definition types supported and fields within those definitions.

## MusicTrack
This data structure defines a sequencer (non-CDDA) music track. It specifies the track number (which is referenced by maps) and the sequence index within the module file (.WMD) for that track. This could be used to add new music tracks, provided they have been added to the .WMD file. Example:
```
MusicTrack 1  { Sequence = 90  }    // The same as the built-in track '1', can modify the existing definition if different
MusicTrack 31 { Sequence = 136 }
MusicTrack 32 { Sequence = 137 }
```
