# PsyDoom MAPINFO documentation
Table of contents:
- [Overview](#Overview)
- [`Map` definition](#Map-definition)
- [`Episode` definition](#Episode-definition)
- [`ClearEpisodes` definition](#ClearEpisodes-definition)
- [`Cluster` definition](#Cluster-definition)
- [`GameInfo` definition](#GameInfo-definition)
- [`MusicTrack` definition](#MusicTrack-definition)
- [`CreditsPage` definition](#CreditsPage-definition)
- [`ClearCredits` definition](#ClearCredits-definition)

## Overview
To help modding, PsyDoom supports a 'MAPINFO' lump which allows information like level & episode names, music selection and so on to be defined. The purpose of this document is to list the definition types supported and fields within those definitions.
 
Some high level points to note:
- The lump must be defined in a main IWAD file (not a map WAD file). Normally you would use the extension IWAD `PSXDOOM_EXT.WAD` to do this. It can also be present in `PSXDOOM.WAD`.
- The syntax used is similar to the 'New ZDoom' format (see: https://zdoom.org/wiki/MAPINFO).
- All block and field names are case insensitive. Camel case is recommended however for readability.
- The MAPINFO lump is processed sequentially, from start to finish. This means that definitions like `ClearEpisodes` should be placed first before `Episode`.
- Unrecognized definition types or fields will be ignored.
- If the game already defines something (Map, Episode etc.) then defining it again in MAPINFO overwrites the existing definition. Furthermore, if some fields are omitted from the new definition, then those fields will be sourced from the original object. In this way you can use MAPINFO to partially tweak some existing game data, without having to redefine all of it. For example, you could alter the music track of an existing map without changing anything else.

## `Map` definition
Defines various settings unique to each map. Full example (not all these fields need to be specified):
```
Map 3 "My Cool Map" {
    Music = 5
    Cluster = 1
    SkyPal = -1
    PlayCdMusic = 0
    NoIntermission = false
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
- `SkyPal`: Can be used to override the palette used for the map's sky. Doom has 20 built-in palettes; Final Doom has 26. The engine can support up to 32 palettes in PLAYERPAL, so 6 additional user palettes can be defined. Specify `-1` (the default) to have the engine automatically determine the sky palette from the sky texture number. This field must be between -1 and 31. Built in palettes are as follows:
    ```
    MAINPAL               = 0       // Used for most sprites and textures in the game
    REDPALS (0-7)         = 1-8     // Pain palettes (red shift)
    BONUSPALS (0-3)       = 9-12    // Bonus pickup (gold shift) palettes
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
- `PlayCdMusic`: If `1` or greater then `Music` is interpreted as a CD track to play instead of a music sequence. You can use this to play CD audio for the map instead of sequencer music. Default is value is `0`.
- `NoIntermission`: If `1` or greater then the intermission screen is skipped for this map. Default is value is `0`.
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
## `Episode` definition
Defines an episode which can be selected on the main menu. Note: the game's episode list should be contiguous, with no missing episode numbers between the first and last. Example:
```
Episode 1 { 
    Name = "Hell To Pay!"
    StartMap = 1
    LogoPic = "DOOM"
    LogoPal = -1
    LogoX = 75
    LogoYOffset = 0
    IsHidden = false
}
Episode 2 { 
    Name = "Doomed..."
    StartMap = 10
}
```
Header fields:
- `1st`: The episode number which identifies the episode. Must be between 1 and 255.

Internal Fields:
- `Name`: Name of the episode which is displayed on the main menu.
- `StartMap`: Which map to load when starting a new game with this episode selected. Must be between 1 and 255.
- `LogoPic`: which lump to show for the episode logo. Defaults to `DOOM` if not specified.
- `LogoPal`: which palette to use for the episode logo. Should be 0-31 or `-1` to use whatever palette is defined globally for the `DOOM` graphic. Defaults to `-1` if not specified.
- `LogoX`: The horizontal/x position to display the episode logo at on the main menu. Defaults to `75` if not specified (original position for Doom & Final Doom).
- `LogoYOffset`: An additional y offset that can be added to the episode logo's position when rendering on the main menu. Defaults to `0` if not specified.
- `IsHidden`: if `true` then the episode is not a selectable single player episode (used to define an episode logo for secret maps in multiplayer only). The default value is `false` (normal episode).

## `ClearEpisodes` definition
When this definition is encountered it instructs the game to clear the current list of episodes. You can use it for example to remove unwanted episodes from Doom or Final Doom. Note: the definition doesn't have any other information associated with it, it's just a simple command/instruction:
```
ClearEpisodes {}
```
## `Cluster` definition
Defines the finale to show for a group of maps. Whenever the game detects the last (non-secret) map is completed or the next map is from a different cluster then the finale screen will be shown. An example, which matches the settings for the Doom II cluster:
```
Cluster 2 {
    CastLcdFile = "MAP60.LCD"
    Pic = "DEMON"
    PicPal = 0
    CdMusicA = 8
    CdMusicB = 4
    TextX = 0
    TextY = 45
    SkipFinale = false
    HideNextMapForFinale = false
    EnableCast = true
    NoCenterText = false
    SmallFont = false
    Text = 
        "you did it!",
        "by turning the evil of",
        "the horrors of hell in",
        "upon itself you have",
        "destroyed the power of",
        "the demons.",
        "their dreadful invasion",
        "has been stopped cold!",
        "now you can retire to",
        "a lifetime of frivolity.",
        "congratulations!"
}
```
Header fields:
- `1st`: The cluster's number. Must be between 1 and 255.

Internal Fields:
- `CastLcdFile`: Which LCD files contains the SFX used for the cast call. In the retail games this is 'MAP60.LCD'.
- `Pic`: Which image to show for the finale background.
- `PicPal`: Which palette to use to display `Pic`. Must be between 0 and 31. For a list of available palettes, see the documentation for `Map`.
- `CdMusicA`: The CD track to play once for the finale.
- `CdMusicB`: The CD track which will loop after `CdMusicA` is finished.
- `TextX`: X position of finale text. Ignored unless `NoCenterText` is enabled (`true`).
- `TextY`: Y position of the finale text.
- `SkipFinale`: if set to `true` then no finale will show.
- `HideNextMapForFinale`: if the conditions to show a finale arise (ignoring `SkipFinale`), hide the 'Entering <MAP_NAME>' message and password on the intermission screen?
- `EnableCast`: if set to `true` then a finale with a cast call will be shown.
- `NoCenterText`: if set to `true` then the finale text will not be centered.
- `SmallFont`: if set to `true` then the small (8x8) font used for rendering status bar messages will be used for the finale.
- `Text`: The text to show for the finale, with one line in each string. Each pair of lines is separated by `,`.

## `GameInfo` definition
Defines general (global) game settings. If these settings are not explicitly specified, then the default values for the current game disc will be used. Example, using the same settings as the original PSX Doom:
```
GameInfo {
    NumMaps = 59
    NumRegularMaps = 54
    DisableMultiplayer = false
    FinalDoomGameRules = false
    AllowWideTitleScreenFire = false
    AllowWideOptionsBg = false
    TitleScreenStyle = 0
    CreditsScreenStyle = 0
    TitleScreenCdTrackOverride = -1
    TexPalette_TitleScreenFire = 15
    TexPalette_STATUS = 16
    TexPalette_TITLE = 17
    TexPalette_TITLE2 = 17
    TexPalette_BACK = 0
    TexPalette_Inter_BACK = 0
    TexPalette_LOADING = 16
    TexPalette_PAUSE = 0
    TexPalette_NETERR = 0
    TexPalette_DOOM = 17
    TexPalette_CONNECT = 0
    TexPalette_OptionsBG = 0
    TexLumpName_STATUS = "STATUS"
    TexLumpName_TITLE = "TITLE"
    TexLumpName_TITLE2 = "TITLE2"
    TexLumpName_BACK = "BACK"
    TexLumpName_Inter_BACK = ""
    TexLumpName_OptionsBG = "MARB01"
}
```
Internal Fields:
- `NumMaps`: the total number of maps available in the game, including secret ones. For Doom and Final Doom this value was `59` and `30` respectively. Affects which map numbers are valid to cheat warp to etc. Must be between 1 and 255.
- `NumRegularMaps`: the number of non-secret maps in the game. Any maps after the last non-secret map are assumed to be secret maps. Affects when the game detects the last (normal) level of the game has been completed and which levels can be used as the starting point for a multiplayer game. For Doom and Final Doom this value was `54` and `30` respectively. Must be between 1 and 255.
- `DisableMultiplayer`: whether multiplayer game modes should be disallowed. Disabling multiplayer can be useful for mods that do tricks like Doom 64's faux-3D bridges, or 'silent teleporters' which would break in a multiplayer game.
- `FinalDoomGameRules`: whether Final Doom style game rules should be used. Final Doom forward movement is slightly slower, and only 16 Lost Souls can spawn from Pain Elementals.
- `AllowWideTitleScreenFire`: if enabled then the fire sky on the title screen can tile/repeat taking widescreen into account (beyond the original UI area).
- `AllowWideOptionsBg`: if enabled then the options menu background can tile/repeat taking widescreen into account (beyond the original UI area).
- `TitleScreenStyle`: what style of title screen to use. Valid values are currently:
  - `0` = Doom: the 'DOOM' logo rises up above the fire.
  - `1` = Final Doom: the 'TITLE' image displays over the fire at all times and fades in from black.
  - `2` = GEC Master Edition Beta 3: the `DOOM` and `FINAL` images display side-by-side over the fire at all times, fading in from black. The 'Master Edition' text from the 'DATA' lump also displays on top.
  - `3` = GEC Master Edition Beta 4: show transparent fire over the `COVER` image and then the logos from `TITLE2` on top of that. Fade in the background and logos.
- `CreditsScreenStyle`: what style of credits screen to use. Valid values are currently:
  - `0` = Doom: 2 pages, ID and Williams.
  - `1` = Final Doom: 3 pages, level, Williams and ID credits.
  - `2` = GEC Master Edition: 5 pages, GEC, Doomworld, and Final Doom credits.
- `TitleScreenCdTrackOverride`: if >= 2 then play this CD track on the title screen instead of the default one.
- `TexPalette_TitleScreenFire`: palette index to use for the fire on the title screen. Must be between 0 and 31.
- `TexPalette_STATUS`: palette index to use for the `STATUS` image lump. Must be between 0 and 31.
- `TexPalette_TITLE`: palette index to use for the `TITLE` image lump. Must be between 0 and 31.
- `TexPalette_TITLE2`: palette index to use for the `TITLE2` image lump (GEC ME specific). Must be between 0 and 31.
- `TexPalette_BACK`: palette index to use for the `BACK` image lump. Must be between 0 and 31.
- `TexPalette_Inter_BACK`: palette index to use for the `BACK` image lump on the intermission screen specifically. Only used if `TexLumpName_Inter_BACK` is defined. Must be between 0 and 31.
- `TexPalette_LOADING`: palette index to use for the `LOADING` image lump. Must be between 0 and 31.
- `TexPalette_PAUSE`: palette index to use for the `PAUSE` image lump. Must be between 0 and 31.
- `TexPalette_NETERR`: palette index to use for the `NETERR` image lump. Must be between 0 and 31.
- `TexPalette_DOOM`: palette index to use for the `DOOM` image lump. Must be between 0 and 31.
- `TexPalette_CONNECT`: palette index to use for the `CONNECT` image lump. Must be between 0 and 31.
- `TexPalette_DATA`: palette index to use for the `DATA` image lump (GEC Master Edition addition). Must be between 0 and 31.
- `TexPalette_FINAL`: palette index to use for the `FINAL` image lump (GEC Master Edition addition). Must be between 0 and 31.
- `TexPalette_OptionsBG`: palette index to use for the options menu tiled background. Must be between 0 and 31.
- `TexLumpName_STATUS`: which texture lump name to load wherever the `STATUS` texture is used.
- `TexLumpName_TITLE`: which texture lump name to load wherever the `TITLE` texture is used.
- `TexLumpName_TITLE2`: which texture lump name to load wherever the `TITLE2` texture (GEC ME specific) is used.
- `TexLumpName_BACK`: which texture lump name to load wherever the `BACK` texture is used.
- `TexLumpName_Inter_BACK`: which texture lump name to load for the `BACK` texture on the intermission screen specifically. If unspecified (empty string) then `TexLumpName_BACK` will be used instead.
- `TexLumpName_OptionsBG`: which texture lump name to load for the options menu tiled background.

## `MusicTrack` definition
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

## `CreditsPage` definition
Defines a single credits page/screen. The pages will appear in the order that they are defined. Examples:
```
CreditsPage {
    BgPic = "IDCRED1"
    FgPic = "IDCRED2"
    BgPal = 18
    FgPal = 16
    FgXPos = 9
    MaxScroll = 182
    FgAdditive = false
}
CreditsPage {
    BgPic = "WMSCRED1"
    FgPic = "WMSCRED2"
    BgPal = 19
    FgPal = 16
    FgXPos = 7
    MaxScroll = 228
    FgAdditive = false
}
```
Internal Fields:
- `BgPic`: Which lump to show for the background of this credits page.
- `FgPic`: Which lump to show for the foreground/text of this credits page.
- `BgPal`: which palette to use for the background. Should be 0-31.
- `FgPal`: which palette to use for the foreground/text. Should be 0-31.
- `FgXPos`: which x position to display the foreground/text image at. Default value is `0`.
- `MaxScroll`: how many pixels up should the text scroll before switching to the next credits page. Default value is `256`.
- `FgAdditive`: if 'true' then the foreground/text is blended additively against the background. Default value is `false`.

## `ClearCredits` definition
When this definition is encountered it instructs the game to clear the current list of credit pages. You can use it to create an entirely new set of credit pages from scratch. Note: the definition doesn't have any other information associated with it, it's just a simple command/instruction:
```
ClearCredits {}
