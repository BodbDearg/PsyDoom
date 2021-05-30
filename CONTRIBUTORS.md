PsyDoom contributors (sorted alphabetically)
============================================

* **[Erick Vásquez García (Erick194)](https://github.com/Erick194)**

    * Author of the sister project of PsyDoom: [PSXDOOM-RE](https://github.com/Erick194/PSXDOOM-RE)
        * This project which completely recreates the PSX Doom source code for the original hardware serves as an invaluable source of information for this project. It helped to greatly accelerate PsyDoom's transition to native C++.
        * Also, for the reversed Final Doom code also see: [PSXFINALDOOM-RE](https://github.com/Erick194/PSXFINALDOOM-RE)
    * Bug fixes, including:
        * "R_InitTextures & R_InitFlats : fix loop issues spotted by @Erick194"
            https://github.com/BodbDearg/PsyDoom/commit/67b693aef8bf1ea71fe564b82395db9af9554742
    * Various information and code for PSX DOOM based on his own extensive reverse engineering efforts, including:
        * Z_Zone comparison:
            https://gist.github.com/Erick194/cccab97eb7cad64fb338fa1048a4743d
            https://gist.github.com/Erick194/2469d4bc1c04748b60e87235885c1977
        * P_LoadBlocks comparison:
            https://www.doomworld.com/forum/topic/111024-stationdoom-reverse-engineered-source-port-of-psx-doom-for-pc-early-progress/?page=1
        * 'vertex_t' fields and zone memory manager purge flags:
            https://github.com/BodbDearg/StationDoom/commit/a593db0ad7acb693749446e4a7e65756d8c01433
        * Password encoding/decoding:
            https://github.com/BodbDearg/StationDoom/commit/b92176e5755bb55f7a20783281b57b645f8252ca
        * Various enums and constants, 'seg_t' fields, 'buttons_t' fields, 'psxobj_t' fields, fire sky code:
            https://github.com/BodbDearg/StationDoom/commit/38ecd7bebb4ddf3c84002412cb3f83e3f42d45dd
        * P_CheckCheats, sfx numbers, automap control & drawing code:
            https://github.com/BodbDearg/StationDoom/commit/ef070d19c8cdbe808838be577c24ea674308d678
        * P_Base/P_Move naming suggestions, PM_PointOnLineSide, R_RenderSKY:
            https://github.com/BodbDearg/StationDoom/commit/b34c232fd833d5df23d652c6da6353424d62dc23
        * I/W_AddPrim comparison:
            https://github.com/BodbDearg/StationDoom/commit/c3968607c64b82f4165af347303bb84265e524a8
        * FixedMul/Div comparison:
            https://github.com/BodbDearg/StationDoom/commit/bba3955c2c04fba9199929e87de3b7834ce90f98
        * Pretty much the entire PSX renderer!
            https://gist.github.com/Erick194/96f360d99e905f085f0ee50c097d3b65
            https://gist.github.com/Erick194/0f57e686e57ec43f1067f254f2fbe79a
            https://gist.github.com/Erick194/e609690fe0250c5243cb4c705815dacf
        * P_PlayerZMovement
            https://www.doomworld.com/forum/topic/101161-gec-master-edition-psx-doom-for-the-playstation-1102019-beta-3-release-now-are-you-ready-for-more-action/?page=54&tab=comments#comment-2076239
        * Texture cache code:
            https://gist.github.com/Erick194/41c9ee0ac44e3a4c4daa952ac28f1b28
        * Pointing out the origin of 'legals' screen (used in the demo):
            https://github.com/BodbDearg/PsyDoom/commit/77cc15835535ec44f4248c5f55ebdf3aca57bb31
        * Pointing out issues with 'Nightmare' mode passwords and a fix:
            https://github.com/BodbDearg/PsyDoom/commit/53219ed02236d96b3a628bc8db43233928954f8d
            https://github.com/Erick194/PSXDOOM-RE/commit/7d1a37385c32c5c8f3026c3d8164647c8e67ff00
        * Support for extended password map numbers (0-255, instead of 0-63):
            https://github.com/Erick194/PSXDOOM-RE/commit/2a5b73503db5479131702ee29dc946056bcf026a
        * Clarified some telefrag behavior:
            https://github.com/BodbDearg/PsyDoom/commit/7c92be7dd2a5a34f0efa066c4d26384115b3546a#r51501054

* **[fenderc01](https://github.com/fenderc01)**

    * Found and raised an issue with a 'ptrToVmAddr' error:
         https://www.doomworld.com/forum/topic/111024-psydoom-060-psx-doom-port-reverse-engineered-for-pc/?do=findComment&comment=2121606
    * Suggestion to make opposite movement directions cancel:
         https://www.doomworld.com/forum/topic/111024-psydoom-060-psx-doom-port-reverse-engineered-for-pc/?do=findComment&comment=2144091
    * Discovered weapon switching issues when switching really quickly:
         https://www.doomworld.com/forum/topic/111024-psydoom-060-psx-doom-port-reverse-engineered-for-pc/?do=findComment&comment=2151142
    * Raised a bug about the invalid password flash overlapping text:
         https://www.doomworld.com/forum/topic/111024-psydoom-060-psx-doom-port-reverse-engineered-for-pc/?do=findComment&comment=2181219
    * Discovered issues with using a PlayStation classic controller:
         https://www.doomworld.com/forum/topic/111024-psydoom-060-psx-doom-port-reverse-engineered-for-pc/?do=findComment&comment=2224903
    * Spotted a saved_prefs.ini handling inconsistency:
         https://github.com/BodbDearg/PsyDoom/issues/1
    * Made a separate Doom/Final-Doom password suggestion:
         https://github.com/BodbDearg/PsyDoom/issues/1
    * Found issues with the 'Inv Mouse Wheel' control binding:
         https://github.com/BodbDearg/PsyDoom/issues/3
    * Discovered and a raised bug with text disappearing on Finale:
        https://github.com/BodbDearg/PsyDoom/issues/7
    * Discovered issues creating the Vulkan PSX VRAM texture on an AMD Radeon HD 8790M.
        https://github.com/BodbDearg/PsyDoom/issues/8
    * Updated Open PsyDoom Config Folder.bat:
        https://github.com/BodbDearg/PsyDoom/pull/9

* **[j0r9](https://github.com/j0r9)**

    * Suggestion to interpolate weapon bobbing:
        https://github.com/BodbDearg/PsyDoom/issues/2

* **[Jakub Czekański (JaCzekanski)](https://github.com/JaCzekanski)**

    * Author of the excellent Avocado PlayStation 1 emulator, which PsyDoom used previously as a host and bootstrap environment in addition to emulating specific devices such as the SPU and GPU while the game was being transitioned to a fully native & standalone app:
        https://github.com/JaCzekanski/Avocado

* **Martin Korth**

   * Author of the NO$PSX emulator and also of the NO$PSX specs, which have been extremely helpful during development:
        https://problemkaputt.de/psx-spx.htm

* **[Samuel Villarreal (svkaiser)](https://github.com/svkaiser)**

    * Console DOOM reverse engineering, specs & tools:
        https://www.doomworld.com/forum/topic/38608-the-console-doom-hacking-project-console-specs
    * Doom64-EX source code (DOOM 64 was based on PSX DOOM, thus can serve as a reference point for it):
        https://github.com/svkaiser/Doom64EX
