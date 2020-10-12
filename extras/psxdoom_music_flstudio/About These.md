# About These
These are the sequenced (i.e. non Redbook audio) music tracks from PlayStation Doom and Final Doom which have been exported from the game's tracker-style format and converted to self-contained FL Studio project files. [Audio tools](https://github.com/BodbDearg/PsyDoom/tree/master/tools/audio) developed alongside PsyDoom were used to help perform this conversion.

These files are here for historical/educational reference only, so that the magic behind these wonderful soundtracks can be studied and understood. They should not be used outside of these purposes without prior approval from their original composer, [Aubrey Hodges](http://www.aubreyhodges.com).

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
5. I then replace the placeholder MIDI instruments added by FL Studio for each track with instances of the `PSXSampler` VST instrument. The `PSXSampler` instrument instances are populated with the correct sound sample, pan, volume and ADSR envelope settings etc. for the patch voice, as defined by the .WMD file and viewable through the JSON dump. Note that some patches in the .WMD contain multiple voices (layered patches), so in these cases I use a 'Patcher' instrument in FL Studio to layer multiple `PSXSampler` VSTs into one logical instrument.
6. FL Studio defines the root/middle note by default to be 'C5', which is not correct for this import. It must be set to 'C4' on all the root instruments or otherwise the music will be an octave lower than it should be.
7. To cleanup I get rid of ALL automation/control change data from the imported MIDI pattern, except for pitch bends. I bake the panning control change into the channel pan settings and as well as the track channel volume control change. When baking the volume levels care must be taken to ensure the linear volume multiplier for the channel is correct as the % volume amount is non-linear.
8. I split up the single pattern by track/instrument to make viewing individual tracks easier.
9. To enable correct pitch bend I make sure the 'Send pitch bend range' setting is enabled for all instruments in FL Studio and set to '2'. For more about this see the following thread: https://www.reddit.com/r/FL_Studio/comments/er8e7u/pitch_bend_is_wrong_when_importing_midi_files/
The pitch bend range will be interpreted as +2/-2 semitones in FL Studio when importing a MIDI file and visualized as such. Setting the 'Send pitch bend range' setting to '2' causes pitch bends within that range to be normalized to the entire pitch wheel range before being sent to `PSXSampler` instances. The `PSXSampler` instances will then scale the pitch bend to its own range, depending on the value of the 'Pitchstep Up/Down' settings. These settings define the bend range in semitones.
10. Finally I route all instruments to output to channel 1 and add a `PSXReverb` VST3 effect to this channel. To know which reverb type and reverb depth to use, the PsyDoom source file `s_sound.cpp` is consulted. Once that is done, I tweak the master volume level to make the music more audible.
