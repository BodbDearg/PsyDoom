# Changelist

- [1.0.1](#101), [1.0.0](#100)
- [0.8.3](#083), [0.8.2](#082), [0.8.1](#081), [0.8.0](#080)
- [0.7.4](#074), [0.7.3](#073), [0.7.2](#072), [0.7.1](#071), [0.7.0](#070)
- [0.6.1](#061), [0.6.0](#060)
- [0.5.2](#052), [0.5.1](#051), [0.5.0](#050)
- [0.4.0](#040)
- [0.3.0](#030)
- [0.2.0](#020)
- [0.1.1](#011), [0.1.0](#010)
- [0.0.3](#003), [0.0.2](#002), [0.0.1](#001)

----------------------------------------------------------------
# 1.0.1

## Important note
This release raises the minimum required Windows version to 8.1. Windows XP, 7 and 8.0 are no longer supported.

## Feature changes & improvements
- Add the ability to toggle the 'uncapped framerate' setting in-game via the 'extras' menu and via a toggle key (unbound by default).
    - This makes it easier to switch between high-fps Vulkan rendering and low-fps 'classic' rendering, all while in-game.
    - Note: the old config setting for uncapped framerate is now removed as a result of this change.
- Demo recording: show when recording has started and ended for added clarity.
    - There are some situations like when loading a save game where demo recording is not allowed; this change helps identify those situations.
- Demo recording: allow the game to be paused while recording instead of just quitting the game.
    - Note: pausing causes recording to end, since demos can't pause the game.
    - This change can be helpful for speed-runners to restart the level again and record the next run.
- Simply the HUD messages emitted when loading and saving a game so that they don't clutter up the UI.
- Deathmatch: display a frags count (you/them) if kill stats are enabled.

## Bug fixes
- GEC Master Edition: fix the wrong CD music track playing for the 'Icon Of Sin' level.
- Vulkan renderer: fix level and performance stats not drawing in the right place if widescreen is disabled.
- Doom MAP20 'Unholy Cathedral': fix some lines incorrectly marked as doors, and door tracks that should not move.
- Classic renderer: fix an issue that has the rare potential to cause crashes.

----------------------------------------------------------------
# 1.0.0

## Feature changes & improvements
- Added the ability to load from and save to savefiles.
    - There are 5 save slots available, 3 manual/numbered slots, 1 quicksave slot and one autosave slot.
    - Autosave happens at the start of each level.
    - Each game type also has it's own set of savefiles which do not interfere with each other.
- Added a new GUI based launcher to make configuring and running the game much easier.
- Uncapped framerate: added motion interpolation/smoothing for sectors, map objects, and the player weapon.
    - These can all be disabled if required.
- Implemented the playback of intro logos and movies to mimic the startup flow of the original game.
- Added full support for playing the 'GEC Master Edition Beta 3'.
    - PsyDoom also supports playing the single level test discs produced by the GEC modding tools.
- Added support for Linux.
    - Officially supported architectures are `x86_64`, `arm64` and `armhf`.
    - A Debian 'Buster' based distro (or later) is recommended. Other distros might work, but have not been tested.
- Tweaked interpolation for improved smoothness.
    - Space frames apart by a fixed and absolute amount in time to improve pacing consistency and visual smoothness.
- Dropped the required Vulkan version down to 1.0 instead of 1.1.
- Added a new app icon.
- Implemented demo recording and a new enhanced demo format for PsyDoom.
    - This new format supports recording multiplayer games among other improvements.
    - During multiplayer demo playback, you can also toggle which player is viewed.
- Map patches: add the ability to configure whether certain categories of map patches are applied or not.
    - Most patches can be opted out of, if original bugs are desired.
- Add the ability to type in passwords with the keyboard.
- Implemented onscreen performance counters that can be enabled to profile the game. The counters show average FPS and frame time in microseconds.
- Vulkan renderer: made occlusion culling more lenient to help avoid 'pop in' of sprites when bordering visible and hidden areas. Render more subsectors, at the cost of some performance but hopefully with less pop in.
- Vulkan renderer: added tweaks to flat merging and sprite splitting to try and improve some draw order issues.
- Vulkan renderer: disable MSAA for lower powered GPUs by default. Also use low resolution (480p) on the Raspberry Pi by the default.
- Make nightmare Pain Elementals spawn nightmare Lost Souls. Also make spectre Pain Elementals spawn spectre skulls.
- Config: remove the default cue file path which was 'Doom.cue'. Instead force the user to choose which disc to play with. This should reduce confusion in future. Also make the launcher check that a valid (existing) cue file is chosen.
- Add support for 'Voodoo dolls' that behave largely similar to those found in PC Doom. Limitation: sector damage effects do not currently effect the dolls.
- Video: make whether to use v-sync configurable. Note: this setting might be ignored by the OS/driver.
- Implemented support for playing the one level PAL/Europe demo of 'Doom'.
    - Supports the standalone Doom demo disc as well as the demo found in collections like 'Essential PlayStation 3' and 'Euro Demo 103' (Official UK PlayStation Magazine).
- Implement a tweak which allows multiple 'Computer Area Map' powerups to be picked up (enabled by default).
    - This fixes issues on some maps where 100% items cannot be obtained because there are multiple map powerups present.
- Tweak how co-op indirect monster kills are handled. Instead of assigning the kill to nobody try assigning the kill to the player being targeted by the enemy; this player is most likely responsible for the indirect kill. If this is not possible (e.g due to monster infighting) then randomly assign the kill to one player or another.
- Icon Of Sin: don't spawn an Arch-vile if the sprites for it are not present in the loaded wads. Instead spawn a Revenant.
- Raised the default audio buffer size from 128 to 256 samples to help avoid playback issues.
- Deathmatch: hide HUD kills/secrets/item stats even if turned on in the options menu. They don't make sense in this mode.
- Co-op: make the in-game stats joint counts, so overall level completion can be determined by both players. On the tally screen individual contributions to those counts will still be shown.
- Added `-warp <MAP_NUMBER>` and `-skill <MAP_NUMBER>` command line arguments.
    - These arguments allow quick warping to a specific map for testing - useful for map development.
- Teleporting: disallow all self-telefragging. Added as a safety measure now that multiple line specials can be triggered at once (see bug fixes), in case the player activates two sides of the same teleporter at the same time.
- Sector specials: issue a warning rather than a fatal error if entering a sector with an invalid special.
    - Allows the game to recover from mapping errors in this situation.
- MAPINFO: add a `HideNextMapForFinale` flag to `Cluster` definitions. This flag allows hiding of the next map and password displays on the intermission for the last map in the cluster.
- MAPINFO: made various menu palettes configurable and also made the options menu tile graphic configurable.
- MAPINFO: make the palettes used for STATUS, TITLE and credits related lumps all configurable.
- MAPINFO: allow either a Doom or Final Doom style title or credits screen to be used.
- Added some additional validation and sanity checks to program arguments. Certain combinations of settings contradict each other or are not supported.
- Vulkan renderer: allow 1 sided lines to be rendered masked or translucent.
    - This makes the Vulkan renderer behave the same as the classic renderer in this scenario.
- Tweaked the view bobbing strength fix to make it less jerky/glitchy at low frame rates.
- Add a '-nolauncher' command line argument which skips/bypasses the launcher.
    - This argument has no effect other than skipping the launcher.
    - The launcher is skipped when any command line arguments are provided, the new switch serves as a way to invoke that behavior without having any other side effect.
- LCD loader: abort loading an LCD file if it contains samples not listed in the WMD file and issue a warning. This added safety will prevent crashes and undefined behavior in this situation.

## Bug fixes
- Added various patches for map issues in both Doom and Final Doom.
    - These patches are mostly to address minor visual issues.
    - Also fixed many for 'GEC Master Edition Beta 3', including some progression blockers.
    - All these map fixes are far too numerous to list here!
- Fix line activation logic not being reliable in some cases.
    - Fixes some situations where switches would be hard to use, and also some exploits where switches could be used through walls.
- Implement a fix (enabled by default) to the 'total kills' counter: increment the total whenever a new enemy is spawned. This fix prevents the total kill % from exceeding 100 and allows the player to determine how many enemies are actually left.
- Fix an original bug where sounds sometimes have incorrect reverb applied after starting a level.
- Fix the view angle not interpolating smoothly on the player's death.
- Fix overriding original flats and textures via loaded wads not working for Final Doom format maps.
- Fix audio clicks when stopping sounds suddenly, like missiles being destroyed.
- Fix some types of blood decorations (thing types 79-81) not moving up or down with elevators.
- Vulkan renderer: fix the game freezing in some parts of the game when alt-tabbing out of the game.
- Fix an original Doom bug where sometimes enemy gibs after crushing can block the player. The bug would happen if the enemy is crushed during it's death animation.
- View interpolation: fix a bug where the view height is not snapped correctly after teleporting. Can cause momentary view movement/glitching if teleporting up or down great heights.
- Fix a crash playing demos via the '-playdemo' command where the map has a fire sky.
- Doom: fix unintended holes in the 'GRATE' texture which cause visual artifacts. Patch the texture to fill in the holes.
    - Note: this fix is enabled when visual map patches are enabled.
- Fix an original issue where two of the music tracks in 'Final Doom' stop eventually and don't play again.
- Fix an original bug where only 1 line special can be crossed per frame.
    - Now any amount can be crossed, if the fix is enabled.
    - Should be helpful for new maps that place many line specials close together.
    - It is still disallowed to cross multiple lines with the same special and tag in 1 frame however...
- Fix an original bug where sometimes sprites on the edge of ledges 'warp' between the top and bottom of the ledge.
- Fix an original PSX crash which happens when the player tries to activate a 'door' special on a 1-sided line.
- Vulkan renderer: fix occasional precision issues rendering sky walls that cause them not to cover intended areas sometimes.
- Vulkan renderer: fix the sky being stretched when widescreen mode is disabled.
- Co-op/dm: fix an original bug where sector specials that cause pain also change the status bar face for the other player. Now the pain face will only be shown for the player receiving the pain.
- Deathmatch: fix negative frag counts not displaying correctly.
- Co-op: fix key pickup sounds not playing.
- Passwords: fix a bug that allowed passwords with invalid map numbers to be entered for 'Final Doom'.
- Vulkan/classic renderers: fix sky walls sometimes not rendering in the correct locations.
- Vulkan device selection: fix a bug where CPU devices were preferred over integrated GPUs.
- Vulkan renderer: fix a rare draw order bug when viewing things at 90 degree angles.
- Fix developer cheats causing the view to jitter sometimes when used.
- PAL: added fixes to interpolation to account for variable game tick durations. This makes interpolation smoother for the PAL case, or at least as smooth as it can be. It can never be perfect since a game tick doesn't have a fixed duration for the PAL case :(
- Fix potential undefined behavior and crashes caused by invalid references between map objects. This issue could arise for example if a monster targets another monster which then gets deleted from the game. Implement a 'weak pointer' system to resolve this problem.
- Config: fix the handling of digit characters in for cheat key sequences.
- Arch-vile: fix the possibility of 'ghost monsters' being created if the arch vile tries to raise a crushed enemy. Crushed enemies can now no longer be raised.
- Sequencer: restrict the maximum amount of time that can be advanced per tick to 0.5 seconds. Prevents some playback weirdness if the app is paused while debugging or interrupted by the OS etc.
- Fixed bullet puffs spawning below the floor. Causes things to look weird for the puff's first frame when interpolation is enabled, because the puff Z position will be clamped then and will jump suddenly. Also guard against the same thing happening with the puff and the ceiling.
- Fixed some default control bindings not mapping as intended.

----------------------------------------------------------------
# 0.8.3

## Bug fixes
- Fix a crash when activating god mode after death, deactivating and then firing.
- Fix original demos potentially de-syncing if MAPINFO specifies Doom vs Final Doom game rules.
- Big font: fix the uppercase letter 'C' being slightly cut off.
- MAPINFO: fix a bug where setting the 'Text' field in 'Cluster' would not clear unused lines.
    - The bug would result in unwanted text from the base game's finale displaying - now fixed.

----------------------------------------------------------------
# 0.8.2

## Bug fixes
- Arch-vile: fix resurrected monsters not having their blending mode preserved.
- Classic renderer: fix occasional numerical overflow when using dual colored lighting.
    - The bug would result in strange wall colors in some situations.
- Cheats: add a message for when the 'X-Ray Vision' cheat is enabled and disabled.
    - Helps explain what is happening to avoid confusion.    
- MAPINFO: fix the 'multiplayer disabled' sound unintentionally playing when moving up and down the main menu.
    - The sound should only play when the user attempts to switch to multiplayer.

----------------------------------------------------------------
# 0.8.1

## Feature changes & improvements
- Implement the option to extend the player shoot range (on by default).
    - If enabled, extends the following player attack limits:
        - Max shoot/hitscan distance, from '2048' to '8192'.
        - Max auto-aim distance, from '1024' to '8192'.
        - Max BFG spray/tracer distance, from '1024' to '2048'.
- Robustness: gracefully handle orphaned lines with no sectors in the map data instead of crashing.
- MAPINFO: add the ability to disable multiplayer for a user mod.
    - May be required if the mod uses tricks that are incompatible with a multiplayer environment.
- Improve the behavior of the Final Doom 'no repeat' mid wall flag.
    - Instead of forcing the mid wall to be 128 units in height the flag now forces the wall height to be the same height as the texture.
    - This allows fence textures smaller than 128 pixels in height to be used.
    - The change doesn't affect original maps, but will be useful for modding.
- Implement sector special '32' (strobe hurt) which behaves the same as special '4' on PC.
    - The PSX version of special '4' is missing the strobe, and this behavior must be preserved for map compatibility reasons.
    - For a PC style 'strobe hurt' special '32' can now be used, if desired.
- Sector special 11 (Damage 10/20% and End level): don't play the player's death sound (silent death).
    - This makes the behavior more like PC.

## Bug fixes
- Classic renderer: fix tall sprites like tech pillars sometimes vanishing up close.
    - These pillars can be found in 'MAP01: Attack' of Final Doom.
- Doom MAP04 (Command Control): fix some steps appearing black.
- Doom MAP22 (Limbo): fix a step/lower-wall not rendering.
- Fix flats and textures in PSXDOOM_EXT.WAD not overriding those in PSXDOOM.WAD.
- Fix an original bug where sometimes sound propagates through closed doors that should block it.
    - This bug can be observed with the small window into the secret room, in MAP03 of Doom.
    - It allows sound to pass through it even though it is closed.
- Fix an original bug where the 'lower and change texture' special doesn't work sometimes.
    - The bug can happen if the sector being lowered is surrounded by another sector that isn't at the destination height.
    - Without this fix a series of lowering floor segments like E3M1 in PC Doom will not work.
- Final Doom: fix incorrect default palette selection for SKY02-SKY06 if a user map with the .WAD extension is loaded.
    - Palette selection logic was incorrectly using DOOM palette selection logic instead of Final DOOM logic.
    - The fix makes the palette selection method vary depending on the base game loaded rather than the map file extension.
- Fix being able to use again scripted 'once only' switches.
- Fix overflows in shooting logic near the minimum & maximum possible map coordinates.
    - Helps prevent strange behavior for larger maps that have areas close to these coordinates.

----------------------------------------------------------------
# 0.8.0

## Feature changes & improvements
- Expanded the available VRAM for the game from 1 MiB to a maximum of 128 MiB (the default).
- Greatly expanded the amount of main RAM available to the engine; it's now 64 MiB by default (up from around 1.3 MiB).
    - This can also be increased further via config settings, if needed.
- Added support for any power of two texture size for walls and flats from 2x2 up to a maximum of 1024x512.
    - Previously only wall textures with pixel widths 16, 64 and 128 were possible.
    - Previously only 64x64 pixel flats were allowed.
- Classic renderer: now allowing up to 8192 sprites (instead of 64) to be drawn per subsector.
- Removed all limits on:
    - The maximum map lump size (was 64 KiB).
    - The maximum number of flats in a map (was 16).
    - Classic renderer: the number of draw subsectors (was 192).
    - Classic renderer: the amount of geometry that can be clipped (was 20 edges and 32 vertices max).
    - The number of scrollable lines (was 32).
    - The number of moving floors (was 30).
    - The number of moving ceilings (was 30).
    - The number of specials that can be triggered in one frame by crossing lines (was 8).
    - The number of switches that can be activated at once (was 16).
    - The maximum size of the music sequencer's .WMD file (was around 70 KiB in previous builds).
- Implemented Doom 64 style dual colored lighting.
    - Due to the need for binary map format compatibility, the feature is slightly more limited than D64.
    - However it does have additional tweaks & controls available that aren't found in the D64 engine.
- Implemented the ability to have floor skies.
- Implemented a new 'invisible/ghost' platform flag for sectors.
    - This makes the sector render as if it's floor height is the same as the lowest sector floor height around it.
    - Can be used to make invisible platforms, like the 'self-referencing sector' technique for the PC engine.
- Implemented support for a new 'MAPINFO' lump to allow user maps and episodes to be named etc.
    - See PsyDoom's modding docs for more details.
- Implemented the ability to add custom animated textures and flats via the 'PSYFANIM' and 'PSYTANIM' lumps.
- Added the ability to define new switch types via the 'PSYSWTEX' lump.
- Implemented a simple 'DECORATE' style lump for adding new (non-interactive) decor sprites.
- Implemented the ability to use Lua for scripted line and sector specials.
    - Scripts allow more control over specials and for elaborate sequences of events to be scheduled.
    - See PsyDoom's modding docs for more details.
- Implemented the ability to adjust floor and ceiling texture offsets via scripting.
    - Can be used to do scrolling effects.
- Reimplemented the following missing enemies/things from PC Doom II:
    - Arch-vile (now DoomEd number '91' for PSX).
    - Wolf-SS.
    - Commander Keen.
    - All 'Icon Of Sin' related things.
- Reimplemented the following missing line specials from PC:
    - 131 & 132: 'raise floor turbo' switch, once and repeatable.
    - 130 & 129: 'raise floor turbo' line cross trigger, once and repeatable.
    - 128: repeatable 'raise floor to nearest' line cross trigger.
- Reimplemented the following sector specials missing from PC:
    - 11: E1M8 style 'death' exit
- Made the 'Texture Cache Overflow' error a non-fatal warning.
    - Graphical corruption may still happen when this situation arises, but using a warning allows the game to recover.
- Implemented an optional on-screen stat counter that can show kills, secrets and item stats.
    - This can be toggled in the 'Extra Options' menu.
- Added logic to de-duplicate and merge identical sounds played at the exact same time to keep audio levels under control.
- Removed the use of the 'TEXTURE1' and 'SPRITE1' lumps by the engine.
    - This makes adding new textures and sprites much easier.
    - Instead use the size and offset info in the texture header.
- Now generating the list of sprites in the game based on main WAD contents rather than hardcoding.
    - Allows new lumps to be added to main game IWADS without breaking the lump numbers in the sprite list.
    - Also allows new sprites to be more easily defined.
- Lua: add the ability to have an external camera viewing something for a brief period.
    - Useful for showing the result of using a switch etc.
- Define new hint flags to specify whether a sky wall should be drawn above a 2-sided linedef.
    - Allows the mapper to control the behavior and fix certain problem cases if required.
- Vulkan renderer: allow lower and upper walls to extend past sector floors and ceilings.
    - Allows certain special effects to be achieved when used in conjunction with sky floors or ceilings.
        - In order to support this change renderer batching must be broken in some (rare) situations.
    - This change also makes the renderer behavior/results more consistent with the classic renderer.
- Added PsyDoom specific generic marker things, intended for use in scripting.
- Overhauled WAD management to allow for more than one main IWAD.
    - PsyDoom will now load 'PSXDOOM_EXT.WAD' and 'PSX_MISSING_ENEMIES.WAD' from the mod directory specified via the '-datadir <MY_DIRECTORY_PATH>' command line argument.
    - The user can also manually add extra IWADs via the '-file <WAD_FILE_PATH>' program argument.
    - For more on all this, see PsyDoom's modding documentation.
- Limit removing: allow sprites with odd widths to be used (normally these would display corrupt).
- Level loading: now handling missing textures more gracefully.
    - Missing textures are replaced with a default and a warning is issued after level startup.
- Tweaked audio compression further to reduce the amount of compression.
    - Should now only activate when sounds get really loud.
- Vulkan renderer: add a (default enabled) tweak that brightens automap lines to compensate for them appearing perceptually darker, due to their thinness at high resolutions.
    - This tweak is required due to a correction made to the automap line colors for both renderers.
    - Previously they were over bright for both renderers due to a bug, so dimness not an issue.
- File overrides mechanism: make file name matching case insensitive.
- Add a developer cheat to automatically re-load a map 'in-place' when it has changed.
    - This reloads the map but preserves player position and view angle. Useful for quickly previewing changes.

## Bug fixes
- Fix a bug where sometimes sound would not work on startup.
- Original bug: fix bullet puffs not appearing sometimes when shooting certain walls outdoors.
- Original bug: LCD Loader: fix a bug that could sometimes cause sounds to be cut short.
    - This bug can be observed on Final Doom MAP28 (Baron's Lair) with the Revenant's pain sound being cut short.
- Automap: fix some colors being slightly incorrect versus how they appear in the original PSX game.
- Windows: fix the game (unintentionally) running at lower resolutions when the Windows display scale is not at 100% (thanks @aybe!)
- Original bug: fix the 'House Of Pain' hidden door bug by applying a map patch.
- Fix music in Doom MAP04 having notes that are slightly off; fix a slight difference in pitch calculations versus the original game.
- Fix lights in a room not going fully out in Final Doom MAP08, Minos, after collecting the Super Shotgun.
- Final Doom MAP23 (Ballistyx): fix the altar ceiling hole not being see through and fix some outer walls dissapeering in the yellow key cage area.
- MAP47 The Citadel: fix not being able to see over the small starting hut even though it is lower than other buildings.
- Original bug: fix numeric overflows in the line-of-sight calculations that would sometimes cause enemies to not see the player.
    - These overflows occurred when sector floor and ceiling heights differed greatly.
- Fix a strange sound playing when returning from the finale to the main menu.
- Fix the item bounding box display in the VRAM viewer.
- Fix view height interpolation being broken when a platform underneath the player is moving but the player is not touching it.
- Fix the engine crashing without explanation if certain required/non-optional textures are not found.
    - Instead issue an error explaining what is wrong.
- Tweak the rocket blast fix to the correct forces applied in some situations and to try and ensure the explosion is always rendered.
- Original bug: fix certain 'raise platform' actions not fully finishing when the target floor height was reached.
    - This bug would prevent further specials from being executed on sectors, because movers were still regarded as 'active'.
- Classic renderer: fix invalid handling of textures that are uncompressed for wall segment rendering (resulted in crashes).
- Classic renderer: fix numerical overflows in sky wall rendering in some cases when really close to sky walls that are very high overhead.
- Classic renderer: fix numeric overflows in extremely tall maps with far offscreen walls.
- Vulkan renderer: fix flickering when sprites are placed at exactly the same location.
- Vulkan renderer: fix a slight error in setting up texture wrapping for the sky (could affect custom skies).
- Final Doom: fix the revenant missile fire sound sometimes playing upon reaching the MAP30 finale.
- Audio tools fixes and error handling tweaks:
    - LcdTool: show more info when there is a sample size mismatch with the WMD when building the LCD file.
    - Fix the handling of implicit blocks in the VAG file. Need to be aware of the file size and zero any blocks that are not in the file.

----------------------------------------------------------------
# 0.7.4

## Feature changes & improvements
- Add a new '-turbo' command line parameter that enables the 'turbo mode' cheat.
    - Turbo mode allows the player to move and shoot twice as fast, and doors and platforms also operate twice as fast.
- Vulkan renderer: draw an extended in-game status bar for widescreen mode, using the existing HUD assets.
    - Can be disabled however if letterboxing is preferred.
- Allow an input key to execute a 'pause' and 'menu back' action at the same time.
    - This allows a key such as 'Escape' to go directly from gameplay to the options screen, bypassing the intermediate 'pause' screen.
    - Change the default key bindings also so that 'Escape' goes directly to the options menu.
- Vulkan renderer: add a small tweak to make sky scrolling smoother.
- Engine limit removing: no longer load .IMG files (containing sprites and textures) for maps, instead load resources from WAD files.
    - This makes modding easier since it removes the need to produce these files.
    - We also don't need to worry about lump number references in .IMG files being invalidated if we add new lumps to the main IWAD.
- Classic renderer limit removing: fix view corruption for tall cliffs of 1024 units or more in height; fix a numerical overflow issue.
- Spu limit removing: extend SPU RAM to 16 MiB by default and make the amount configurable.
    - Will allow any monster variety on a map without worrying about exceeding SRAM limits, and plenty of SRAM for new music and SFX etc.
- Spu limit removing: extend the number of available voices from 24 to 64.
    - This should allow PsyDoom to handle even extremely busy scenes and music without dropping any sounds.
- Spu limit removing: upgrade audio processing and mixing from 16-bit to floating point quality.
    - Improves precision and eliminates distortion on loud sounds caused by audio clipping.
- Spu: add dynamic range audio compression to limit the strength of very loud sounds.
    - Required since audio loudness is almost unlimited and does not clip anymore with the float SPU.
    - The compression is very light in most cases and barely noticeable until very loud situations occur.

## Bug fixes
- Fix an exploit where it was possible to move at 2x speed if using analog and digital movement at the same time.
- Fix an original and PSX specific audio bug that could sometimes cause enemy and player sounds to cut out prematurely.
    - The logic for stopping a missile's spawn sound on explode was slightly flawed, and causing other (unintended) sounds to be stopped instead.
- Vulkan renderer: fix a failure to redraw the screen while connecting to a multiplayer game, after resizing the window.
- Debug builds: fix Vulkan validation layer errors on starting a multiplayer game and on encountering an error during a multiplayer game.
- Windows debug builds: fix an error printed to the console about invalid command line arguments when there are no arguments specified.

----------------------------------------------------------------
# 0.7.3

## Feature changes & improvements
- Vulkan renderer: add the ability to specify a preferred GPU device to use, useful for manual selection in multi GPU systems.

## Bug fixes
- Fix graphical corruption issues on AMD GCN 4 cards and possibly other AMD GPUs.
- Fix discontinuities and 'wobbling' of the sky for some scenes, particularly at lower draw resolutions.
- W_ReadMapLump : fix a crash with reading uncompressed map lumps (affects user maps).
- P_LoadSectors : issue a descriptive error when a flat texture is not found rather than crashing.

----------------------------------------------------------------
# 0.7.2

## Bug fixes
- Fix alignment requirements for memory allocations not being met on certain AMD GPUs.
    - May fix the Vulkan renderer not being available on some AMD GPUs.
- Add a fix for a framebuffer synchronization issue which may cause corruption or black screens on some GPUs.
- Small fix to `Open PsyDoom Config Folder.bat` for handling paths with spaces.

----------------------------------------------------------------
# 0.7.1

## Feature changes & improvements
- Issue a descriptive error if a .cue file path is not set.
- MacOS: don't provide a default location for 'Doom.cue' - the path to it must be set explicitly by the user now. Due to OS security restrictions relative paths likely won't work anymore on MacOS.

## Bug fixes
- Speculative fix for the Vulkan renderer not being available on some AMD GPUs.
- Speculative fix for a black screen issue on some AMD GPUs.
- Fix wrong blend modes being used by the Vulkan renderer in some rare circumstances (mostly for newly created user maps).

----------------------------------------------------------------
# 0.7.0

## Feature changes & improvements
- Added an entirely new Vulkan (1.1) based renderer for the game. Its features include:
    - Support for modern resolutions like 1080p, 1440p etc.
    - Widescreen rendering beyond the original aspect ratio.
    - The ability to instantly toggle between the 'Classic' and Vulkan renderers if desired.
    - Emulates the 16-bit lighting & shading of the original game very closely to help preserve the atmosphere.
        - 32-bit shading can also be enabled if desired but is not recommended.
        - Note: some older MacOS hardware may be less accurate with transparency/blending due to the lack of 16-bit framebuffer support.
    - Improved depth sorting for sprites versus the original game.
        - A lot of cases where sprites are 'cut off' by floors and walls have now been fixed.
        - PsyDoom's Vulkan renderer implements a method of splitting up sprites across subsector boundaries proposed by John Carmack, in the 1997 Doom Source release notes.
    - Triangle edge multisample antialiasing (MSAA).
    - Shader MSAA where the hardware supports it, to help remove 'shimmer' and temporal distortions.
    - The ability to render at low resolutions like 240p for an original feel but with some of the old renderer limitations/bugs fixed.
        - Disable MSAA, set 'VulkanRenderHeight' to '240' and 'VulkanPixelStretch' to '1' for an experience close to the PSX renderer.
    - Triple buffering with out-of-date frame discard for lower input latency.
- PsyDoom no longer depends on the Avocado PlayStation emulator.
    - PsyDoom now has its own internal PSX 'GPU' implementation with extended capabilities like 16-bit texture coordinates and new drawing primitives more optimal to Doom.
    - This move also fixes some glitches with pixels flickering and a few other small things.
- MacOS: added native support for the 'arm64' architecture used by M1 Macs.
    - Note: this is untested/experimental as I do not have access to this hardware currently!
- Added graphics settings to emulate CRT overscan and removed a lot of dead space below the in-game status bar by default.
    - These settings can be tweaked to crop as much of the image as desired.
- Added a 'notarget' cheat that causes monsters to not see the player.
    - By default it can be activated by typing 'idcloak'.
- Upgraded the classic renderer to support room heights greater than 256 units without texture stretching.
    - This fixes texture stretching in a few places in the original maps.
    - This fix is always active; it can only be disabled by compiling a custom version of PsyDoom with 'limit removing' features removed.
    - Note: the Vulkan render has this fix always included too.
- Added a setting to control the output resolution (or window size in windowed mode).
- Provided a new menu option to switch renderers and renamed 'Controls' to 'Extra Options'.
- Added framerate uncapped movement to the automap.
    - Scrolling and moving in the automap should now be smoother.
- Added a new 'fast loading' option useful for speedrunning.
    - This setting skips waiting for sounds to finish and crossfades during loading. It makes loading almost instant on modern hardware.
- Add a '-pistolstart' command line switch that forces pistol starts on all levels.
    - This is similar to the switch found in Chocolate Doom.
- Tweaked/refined some control binding defaults to try and make them more optimal out of the box.
- Implemented an (opt-in) gameplay tweak that reduces the initial firing delay for the Super Shotgun.
    - Makes it feel more like the PC SSG and shifts some of the delay to later in the animation sequence so that damage per second is not affected.
    - This tweak will make it easier to use rapidly, however.
- Added an optional level timer that can be used to measure the time taken to complete a level.
    - This may be useful for speed running.

## Bug fixes
- Fixed rendering bugs where sometimes the ceiling and some walls would 'leak' through the sky from adjacent rooms.
    - The bug would occur where rooms adjacent to a sky had higher ceilings.
    - Not much of an issue with the retail game (avoided largely by design) but can be a limitation for custom maps.
    - The fix is always active for the Vulkan renderer and enabled by default (toggleable) for the Classic renderer.
        - Note: in some situations with the Classic renderer the fix may run into precision problems. It works best with Vulkan.
- Fixed a texture cache overflow error on demos ending.
- Fixed a bug with the view snapping back when pausing while turning.
- Fixed a PsyDoom specific bug in the music system that would cause note pitch to be slightly off sometimes during pitch bend.
- Fixed an original bug in the sequencer system that could cause infinite freezes on level transitions.
- Fixed the 'pause' action sometimes not registering in the finale screens.
- Fixed key up/down events being artificially repeated by the OS if keyboard keys are held down.
- Added a fix to prevent Lost Souls from being occasionally spawned outside of level bounds.
- Fixed an original game bug where sometimes items can't be picked up when overlapping other items that are not possible to pickup.
- Fixed a lack of difference in view bobbing strength between walking and running.
    - View bobbing when walking now should be slightly less intense.
- Fixed broken/stuttering motion when playing back PAL demos with uncapped framerates.

----------------------------------------------------------------
# 0.6.1

## Feature changes & improvements
- Audio system: raise the maximum active sequence and track limit to at least `128` (was previously `24`) to help prevent issues with sounds not playing.
    - Allows more hardware SPU voices to be used.
    - Note: this change doesn't raise the PSX hardware voice limit, but often tracks and sequences are 'active' without using any voices.
        - Thus more voices can be used if the track/sequence limit is raised.

## Bug fixes
- Fixed the 'Inv Mouse Wheel' control input not working.
- Fix audio track & sequence slots sometimes being 'leaked' on level end, resulting in some sounds not playing.

*Note: binaries of the 'Audio Tools' for PSX Doom will not be supplied with this build or future builds going forward unless there are any changes which might affect them. Refer to the [0.6.0](https://github.com/BodbDearg/PsyDoom/releases/tag/releases%2F0.6.0) release for the latest build of the Audio Tools if you need them, or build from source.*

----------------------------------------------------------------
# 0.6.0

## Feature changes & improvements
- Greatly improved audio processing and less stutter.
    - Implemented a new standalone emulation of the PlayStation SPU specifically for PsyDoom, based on Avocado's SPU.
        - Eventually (in future) this can also be used to increase SRAM and voice limits, given its increased flexibility.
    - Audio is now fully multi-threaded and can be fed to the sound device on-demand.
        - The audio device doesn't have to wait for the main game thread to respond.
        - This reduces stutter and audio artifacts.
    - CD-DA music handling is much improved and CD-DA music for .cue files with multiple .bin files now works.
- Implemented a new options menu for turn speed multiplier and auto-run.
- Added an 'autorun' toggle button (Caps Lock by default).
    - This can also be enabled via the new 'Controls' menu.
- Implement saving of certain user preferences to 'saved_prefs.ini' in the same folder as all other config.
    - Audio preferences, in-game configurable control settings and last map password are saved.
- Implement certain well-known PC style cheat key sequences such as 'iddqd' etc.
    - For a full list of cheat key sequences, consult the `cheats_cfg.ini` config file. 
- No-clip cheat now carries across levels and god/no-clip cheats carry across if using cheat warp.
- Add a '-nomonsters' command line argument similar to PC Doom
- Implement support for generic joystick controllers not recognized/supported by SDL.
    - The 'PlayStation Classic' USB controller which falls under this category can now be used for example.
        - For how to bind that controller, see: https://github.com/BodbDearg/PsyDoom/blob/master/extras/control_binding_configs/playstation_classic_controller.ini
- Add more descriptive errors on failure to connect for a network game.
    - If clients or games are mismatched, say so.
- Improved the precision of the game's music sequencer slightly.
- Added the ability to configure the audio buffer size, if required, to trade latency for audio stability.
    - The default settings should be good for most systems, however.
- Allow view bobbing strength to be reduced if required.
- P_SpawnMapThing: skip things with DoomEd num '0' - not valid things!
    - But warn about bad things on level start.
    - A change to make the engine more resilient/friendly for modders.
- Allow animated flats and textures to be referred to by animation frames other than the 1st frame in the sequence.
    - This would previously cause crashes, as the engine always expected animated resources to be referred to by the 1st frame in the sequence.
    - If a mapper uses a SLIME02 flat for instance, it now behaves just as if the mapper had used SLIME01.
    - A change to make the engine more resilient/friendly for modders.
- LCD sound loader: issue a warning if out of sound RAM.
- P_LoadBlocks: print more info in the case of an unexpected decompressed file size.
    - May be useful for modders to diagnose issues with loading certain map files.
- Add support for the unused hanging lamp sprite used in the GEC Master Edition (DoomEdNum 90).
- Implemented (separate) command-line audio tools for manipulating the game's music and sound formats.
    - May be of interest to modders of PlayStation Doom!
    - Using these tools I also converted all of the game's music to FL Studio format (Windows 64-bit), see:
        https://github.com/BodbDearg/PsyDoom/tree/master/extras/psxdoom_music_flstudio

## Bug fixes
- Fix gravity being too strong due to the game running at a consistent 30 FPS frame-rate.
    - This is a bug inherited from the original game.
    - This makes the key jump in 'The Mansion' for example now possible, along with other jumps.
- Fix view bobbing not being as strong as it should be, due to a consistent 30 FPS frame-rate.
    - This is a bug inherited from the original game.
- Fix an annoying temporary stutter which would sometimes happen on starting a new game.
- Fix the 'Invalid password' flash overlapping the password.
    - Make the behavior the same as Final Doom - only one or the other shows during the flash.
- Fixed certain instances of undefined behavior in the code which might affect portability to different CPU architectures.
    - Fixed instances of misaligned data-structures.
    - Fixed reliance on undefined behavior of left/right shift for negative numbers.
- Fixed a few instances of the renderer overstepping array bounds (potentially a crash issue).

----------------------------------------------------------------
# 0.5.2

Small improvements to the .cue parser:
- Fix the error messages and provide better info on an invalid INDEX 0/1 entry. Should help with diagnosing .cue file issues.
- Ignore PREGAP and POSTGAP commands instead of issuing an error when they are encountered.
    - PsyDoom should be able to safely disregard these.
- Tweaks to make .cue parser more tolerant of spaces and fully case insensitive.

----------------------------------------------------------------
# 0.5.1

## Bug fixes
- Ultimate Doom: fix warping to secret levels resulting in the finale screen being shown.
- Fix certain on-screen notifications (like NV video recording) leaving marks in fullscreen.

----------------------------------------------------------------
# 0.5.0

## Feature changes & improvements
- Added full support for Final Doom.
    - PsyDoom can now play any variant of PlayStation Doom or Final Doom, PAL or NTSC.
        - Note: with the exception of demos, PAL builds will play using NTSC timings (unless specified otherwise) since the overall user experience is better. PAL style gameplay can be forced via game settings however, if desired.
- Added fully bindable controls.
    - Key mappings can now be changed via the 'control_bindings.ini' configuration file.
- Output scaling/stretch mode can now be configured.
- Doubled the sound buffer size to help prevent audio skipping on some machines.
- Implemented reading of the ISO-9660 filesystem used by PlayStation CD-ROMs.
    - PsyDoom no longer hardcodes the start sectors and sizes of files on disc, as the retail build of PSX Doom originally did.
    - This makes producing a modded PSX Doom CD-ROM image with edited files much easier.
    - The size and location of each game file on disc is now retrieved at startup from the ISO filesystem.
- CD-ROM I/O improvements: removed all CD-ROM emulation except for music playback.
    - File I/O is now direct, much simpler, faster and more reliable.
- Removed the old PlayStation controller configuration screen.
    - This screen no longer serves any purpose in PsyDoom.
    - It may be replaced by some other new in-game menus eventually.
- Classic cheat key sequences can now be entered on the pause menu using an Xbox controller or keyboard.
    - Check your 'control_bindings.ini' file for which keys map to which original PSX buttons.
- Password screen: added the 'exit' option found from Final Doom.
    - The option now appears for both games on this screen.
- Mouse wheel weapon switching improvements: allow movement of more than one weapon slot at a time with the wheel.
- Added Final Doom map format support: allow MAPXX.ROM to be used if MAPXX.WAD is not found.
    - MAPXX.ROM files are interpreted as being in the Final Doom map format, which is slightly different to the original PSX Doom.
        - The binary data structure for sectors and sidedefs differs for Final Doom.
- Added various changes to ensure full demo compatibility with Final Doom and the PAL version of both games.
    - Game settings can be tweaked to use PAL timings and Final Doom's adjustments.
- Expanded PsyDoom's automated demo testing suite considerably.
    - PsyDoom has now verified demo compatibility with PSX Doom and Final Doom for both the PAL and NTSC versions, via 4 complete playthroughs recorded as a collection of demo files against the original games.
    - The only demos which can't be played back precisely at this point are those that generate undefined behavior.
        - Some undefined behavior bugs such as buffer overflows cause unpredictable outcomes that can't be reproduced.
        - PsyDoom fixes all known undefined behavior bugs and replaces them with well-defined outcomes, but it can't reproduce any undefined/unpredictable behavior potentially triggered by original demos.
        - Thankfully however PSX Doom did not have too many commonly triggered undefined behavior bugs, so not a huge issue overall in terms of demo compatibility!
- WMD (Williams Module File) loading: use a dynamically allocated buffer big enough to hold any .WMD file.
    - A stepping stone towards larger custom/modded .WMD files in future.
- Add partial support for the 'PSX Doom Forever' romhack.
    - The game should be mostly be the exact same, but the hacked/modded Russian strings from the binary won't carry over.
    - This will be the extent of PsyDoom's support for this.
- The path to the .cue file used can now be configured/changed.
- Made a movement input latency tweak added for PsyDoom configurable; can disable if original behavior is desired.
- Made the Lost Soul Spawn limit configurable.
- Input: the analog to digital threshold is now configurable.
- I_Error: use a system error dialog for error messages such as 'Texture Cache Overflow'.
    - More user friendly but can be disabled if the original behavior of hanging the game with an error message on-screen is desired.
- Trim some unused stuff from SDL to reduce PsyDoom's binary size slightly.
- MacOS: Metal is now used instead of OpenGL.
    - Should be more forward compatible for future MacOS versions.
- SDL upgraded from 2.0.10 to 2.0.12: contains various bug fixes and improvements.
- Conflicting movement and turning now always cancels.
    - This can be disabled however if required.

## Bug fixes
- Weapon switching: fix a bug where you couldn't immediately switch back to the previously held/ready weapon once another weapon switch was started.
- Fixed interpolation in demos being broken.
- Fix small gaps at the sides of the screen during screen fades.
- Fix not being able to close the game window in windowed mode and make resizing easier.
    - Don't capture the mouse again until the window's client area is clicked.
- Fix a bug where mouse wheel movement would sometimes not register, and possibly other input events.
- Various corner case demo compatibility fixes.
- LCD (sound sample) loader: fixed a bug with trying to read too much data from .LCD files.
- MacOS: the file overrides mechanism for modding now works.
- MacOS: fix a strange freezing issue on pressing the left mouse button (only) just after startup.
- MacOS: fix the mouse wheel not working when shift is pressed.
- Fix undefined evaluation order for the expression 'P_Random() - P_Random()'.
    - Depending on compiler this could produce different results since either side of the '-' can be evaluated first.
    - Make a 'P_SubRandom' function like the Calico DOOM source port to fix.

----------------------------------------------------------------
# 0.4.0

- PsyDoom no longer needs the original PSXDOOM.EXE to run and is now a standalone replacement for the original PlayStation binary.
    - All that is needed to run the game is a valid .cue file (named `Doom.cue`) for the original game disc.
- Added fullscreen support and made it the default video mode.
- Implemented fully uncapped framerates, allowing the game to run at 60 Hz, 120 Hz, 144 Hz etc. 
    - When the framerate is uncapped, player movement and turning is smoothed for frames in-between the original 30 Hz (NTSC) ticks.
    - Enemies and environment objects are not yet smoothed however and still run at 30 Hz, that may be added in future releases.
- Verified that the game can work with any edition of the original PlayStation Doom: NTSC-U, PAL etc.
    - Note: PAL demos do not play back correctly however. The PAL version works correctly other than that.
- Added analog movement support for gamepads.
- Added mouse controls.
- Adjustments to player movement to help reduce input lag:
    - Apply thrust AFTER getting inputs, not before.
    - Tweak is not applied for original demo playback for reasons of compatibility.
- Allow turning to take place at any time (framerate independent) and outside of the normal game loop, which is capped at 15/30 FPS for certain logic.
    - Helps make turning feel more responsive.
- Added a configuration file system (similar to Phoenix Doom) for configuring the game.
    - The list of available options/settings will be expanded in coming releases.
- Main menu : added a 'quit' option (required for users in fullscreen).
- Networking: send the next planned move 1 tick ahead of time to try and workaround latency.
- Networking: try to fix/adjust game clocks for peers who are falling behind in the simulation. Should help keep both players ticking at the same rate, assuming latency is low.
- Keyboard: allow direct weapon switching with the number keys.
- Noclip cheat now has a proper message.
- Floor rendering : added a precision fix (configurable) for cracks in the floor in some of the larger, more outdoor maps.
- Cheat warp menu : allow wraparound to the other end of the map list for convenience
- Incoporate a change from Final Doom to change the default SFX level to 85. Makes music more audible by default.
- Windows : don't show a console window for release builds.
- Removed the temporary '-highfps' hack: now have a proper uncapped framerate mode.

----------------------------------------------------------------
# 0.3.0

- PsyDoom no longer needs a PlayStation BIOS file to run.
- PsyDoom now runs 100% natively and does not emulate the MIPS cpu at all.
- Added support for deathmatch and co-op play, with an emulation of the original link cable functionality over TCP. See the main README.md file for instructions on how to use.
- Verified demo compatibility with the original PSX Doom for a full playthrough of the game. Demos recorded against the original binary play back the same in PsyDoom.
- Password system: nightmare mode now has it's own unique set of passwords. 
- GTE (geometry transformation engine) code is now implemented using regular C++, no emulation used. This removes dependencies on the Avocado MIPS CPU.
- Reduced CPU usage if waiting during the framerate cap.
- Some frame pacing and input lag improvements.

----------------------------------------------------------------
# 0.2.0

- The music sequencer and sound system is now entirely converted to native C++.
- The game can now load .LCD (sound sample) files using the current modding/overrides mechanism. This makes it possible to play new PSX maps with sound, such as those found in "[GEC] Master Edition PSX Doom".
- The game can play now custom maps without having to provide 'MAPSPR--.IMG' and 'MAPTEX--.IMG' files. If these files are not present, the required lumps are loaded from PSXDOOM.WAD instead.
- The music in Club Doom now loops correctly.
- Fixed the first 2 seconds of CD audio tracks being skipped.
- Fix for pause/unpause of cd audio not resuming playback from the previous position.
- Automap: fix a PSX DOOM bug where lines flagged with ML_DONTDRAW would draw when the computer map powerup is obtained.

----------------------------------------------------------------
# 0.1.1

- Fixed issues with the game freezing sometimes on level loads.
- Fixed music not stopping fully on game pause.
- The music sequencer is now driven completely by the host machine's clock, not an emulator clock. As a result music timings and playback should be more accurate. The sequencer logic also no longer runs under emulation and can be debugged normally.
- The lowest levels of cdrom and spu handling (via the PsyQ SDK) are now completely native and talk directly to the emulator 'hardware'. Having control over the interactions with the hardware allowed the game freezing issue to be fixed.
- Spu and cdrom advancement have been completely detached from the rest of the emulated PlayStation and now advance at their own pace and as required. Having isolated spu/cd components will eventually allow me to move sound and music to another thread and make it more reliable.


----------------------------------------------------------------
# 0.1.0

- All rendering and UI code is now implemented pretty much completely natively (i.e regular C++ code) and talks directly to the Avocado rasterizer/gpu. The only remaining work that needs to be done is moving global variables out of the address space of the emulated PlayStation, which I will do near the end of the project. Leaving globals inside the PlayStation's RAM makes it easier to re-emulate certain functions to compare behavior, if required.
- Further improvements to input latency, bypass emulation layers completely to reduce input lag.
- Screen fades now work.
- Improved audio handling, hopefully less stutter.
- Updated Avocado with some fixes for CD music not stopping and some UI sprites rendering 1 pixel too small.
- Title screen: fix a bug from the original game with a 4 pixel gap in the fire at the right side of the screen.
- Allow the 'Nightmare!' skill to be selected from the main menu (note: currently Nightmare passwords are broken)
- Fix not being able to close the window on an I_Error (fatal error message, like "Texture Cache Overflow").
- Add a hack that allows playing the game at 60 Hz instead of the regular 30 Hz max. The hack is enabled by adding the `-highfps` command line switch. Current issues/limitations:
    - View bobbing doesn't work.
    - Gravity being far too strong (related to this physics bug, see: https://www.youtube.com/watch?v=7RBycvyZf3I).
    - Not as smooth as it could be, occasional stuttering.
    - It also doesn't seem to work properly on MacOS.

----------------------------------------------------------------
# 0.0.3

- Window is auto sized based on user screen resolution.
- Window can be resized.
- Game now applies the NTSC scaling that the original game did, stretching the framebuffer from 256 pixels wide to 293 pixels wide. Aspect ratio appears more correct.
- Frame pacing improvements and input handling tweaks to try and reduce input latency.
- Demos now play at the correct speed (15 Hz).
- Xbox 360/One controller can now be used along with analog sticks. Controls are hardcoded at the minute and based on more modern layouts.

----------------------------------------------------------------
# 0.0.2

- Rebranding/renaming to 'PsyDoom'
- Fix a bug with the warp cheat not being shut off after pause menu closed.
- Recompile on Windows with the WinXP SDK - enables running on Win7 and XP.
- Can now quit app by closing window.
- MacOS build - put the required files in the same directory as the .app to use.


----------------------------------------------------------------
# 0.0.1

A VERY early binary (Windows x64) of the project for those who want to try it out.

Please see the main readme file (https://github.com/BodbDearg/StationDoom) for instructions on how to run, as the game has VERY particular requirements at the moment. You need to have *exactly* the correct version of PSX DOOM in order for this to all work, along with a correct PSX bios.

Version commit SHA: d2a192d0c0da8965f8f2e4c80b9c29c22b45c19f 
