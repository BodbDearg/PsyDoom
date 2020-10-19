# About These
These are the sequenced (i.e. non Redbook audio) music tracks from PlayStation Doom and Final Doom which have been exported from the game's tracker-style format and converted to self-contained FL Studio project files. [Audio tools](https://github.com/BodbDearg/PsyDoom/tree/master/tools/audio) developed alongside PsyDoom were used to help perform this conversion.

These files are here for historical/educational reference only, so that the magic behind these wonderful soundtracks can be studied and understood and to allow for personal experimentation with the tracks. They should not be used outside of these purposes without prior approval from their original composer, [Aubrey Hodges](http://www.aubreyhodges.com).

## How to listen to these tracks
FL Studio (Windows 64-bit) is required to play back these pieces along with 2 VST instruments I have created which emulate the PS1 SPU for sample playback and reverb effects. These 2 VST instruments help make the rendition sound very authentic, and close to the in-game performance.

At the time of writing you can download an unlimited trial version of FL Studio from here: https://www.image-line.com

You can also grab releases of the required [PsxSampler](https://github.com/BodbDearg/PlayStation1Vsts/releases) and [PsxReverb](https://github.com/BodbDearg/PlayStation1Vsts/releases) VST3 plugins from the [PlayStation1Vsts](https://github.com/BodbDearg/PlayStation1Vsts/releases) GitHub repository. These are currently only available in VST3 64-bit format for Windows and should typically be copied to the following location to be recognized by FL Studio:
`C:\Program Files\Common Files\VST3`

## The process for generating these files
These files were generated (roughly) by performing the following steps:

1. A dump of the game's DOOMSND.WMD file (Williams Module File) to JSON was performed using the `WmdTool` audio tool found in this repository. This allows me to view what patches are used for each track in each music sequence, and also the settings for each patch. I like to view this data using the helpful tree view of a tool called [JSONEdit](http://tomeko.net/software/JSONedit/).
2. Again using `WmdTool`, a MIDI file for the chosen song is exported from the .WMD file. Music sequences start at index 90 in the .WMD file, anything below that is a sequence for a sound effect. Sequence 90 is music piece 1, 91 is music piece 2 and so on. You can see which sequences are used for particular levels in Doom and Final Doom by consulting the PsyDoom source file, `s_sound.cpp`.
3. Using another audio tool from this repository, `LcdTool`, I export the sound samples for the chosen level's music from the appropriate .LCD file. For MAP01 in Doom for example this .LCD file would be `MUSLEV1.LCD`.
4. Once I have the MIDI and sample data for the music, I import the .MIDI file into FL Studio to a new project.
5. I then replace the placeholder MIDI instruments added by FL Studio for each track with instances of the `PSXSampler` VST instrument. The `PSXSampler` instrument instances are populated with the correct sound sample, pan, volume and ADSR envelope settings etc. for the patch voice, as defined by the .WMD file and viewable through the JSON dump. Note that some patches in the .WMD contain multiple voices (layered patches), so in these cases I use a 'Patcher' instrument in FL Studio to layer multiple `PSXSampler` VSTs into one logical instrument. Also, in order to speed up importing settings, the the 'Params -> Load' functionality in `PsxSampler` can be used with the json data for a patch voice, if it has been saved to a separate .json file.
6. FL Studio defines the root/middle note by default to be 'C5', which is not correct for this import. It must be set to 'C4' on all the root instruments or otherwise the music will be an octave lower than it should be.
7. To cleanup I get rid of ALL automation/control change data from the imported MIDI pattern, except for pitch bends. I bake the panning control change into the channel pan settings and as well as the track channel volume control change.
8. After baking the volume levels into the channel volume settings, the volume amounts must have their scales corrected. FL Studio interprets the MIDI volume level (0-127) as non-linear (volume knob percentage) on import, but PSX Doom interprets the 0-127 amount as a linear multiplier. This difference must be accounted for to get correct in-game volume levels! The channel volume settings need to be converted such that the volume percentage on import equals the volume multiplier after correction. E.G a volume percentage of 70% should be tweaked to become a multiplier of 0.7, which is a volume knob percentage of around 87% in FL Studio. Look to the hint panel in FL Studio to see the multiplier while correcting.
9. I split up the single pattern by track/instrument to make viewing individual tracks easier.
10. To enable correct pitch bend I make sure the 'Send pitch bend range' setting is enabled for all instruments in FL Studio and set to '2'. For more about this see the following thread: https://www.reddit.com/r/FL_Studio/comments/er8e7u/pitch_bend_is_wrong_when_importing_midi_files/
The pitch bend range will be interpreted as +2/-2 semitones in FL Studio when importing a MIDI file and visualized as such. Setting the 'Send pitch bend range' setting to '2' causes pitch bends within that range to be normalized to the entire pitch wheel range before being sent to `PSXSampler` instances. The `PSXSampler` instances will then scale the pitch bend to its own range, depending on the value of the 'Pitchstep Up/Down' settings. These settings define the bend range in semitones.
11. FLStudio has a bug where the start of each imported track does not initialize the pitch bend to '0', even if there is a MIDI command explicitly doing this. This must be manually corrected in the pattern editor or otherwise when rewinding and restarting a track some instruments might be at the wrong pitch level.
12. There is a bug (or intentional behavior?!) with PSX Doom's sequencer whereby whenever a pitch shift downwards is being performed, the pitch is shifted by an extra octave downwards. This is regardless of the 'Pitchstep Down' scaling value and the pitch bend wheel position. To emulate this with `PsxSampler`, the `P.Bend Down Offs.` parameter must be set to `1` for all `PsxSampler` instances. One track where this makes a very noticeable difference is in 'Unhallowed' from Final Doom (used by the map 'Geryon').
13. As a final step I route all instruments to output to channel 1 and add a `PSXReverb` VST3 effect to this channel. To know which reverb type and reverb depth to use, the PsyDoom source file `s_sound.cpp` is consulted. Once that is done, I tweak the master volume level to make the music more audible.

## The process for generating these files: conversion checklist/cheat-sheet
This is just for my own reference; upon converting a track in FL Studio make sure that:
- All volume levels are corrected properly
- All samplers have root note set to C4
- All samplers have 'P.Bend Down Offs.' = 1
- All samplers set pitch bend range = 2
- All control change data removed except pitch bend
- Pitch bend explicitly set to 0 at start of all tracks
- Tracks split up correctly into separate patterns
- Patterns and insturments named appropriately
- Channels named after instruments, each high level instrument routed to its own channel
- 'Main Group' set as the channel group name
- Reverb setup in the master channel
- Master volume level tweaked to be as high as possible without clipping
