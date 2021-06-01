#!python

############################################################################################################################################
# This script builds a 'DOOMSND.WMD' module file with new sequences and instruments added.
# These new tracks and sounds cover re-implemented Doom 2 enemies such as the Arch-vile.
# This .WMD file can be redistributed with user maps to provide the MIDI and instrument data for the new enemy sounds.
#
# Requirements:
#   (1) The PsyDoom audio tool 'WmdTool' must be invokable on the command line (add it to the system path).
#   (2) This script must be executed from this directory.
############################################################################################################################################
import os
import shutil
import subprocess
import sys

# Script paths
WMD_TOOL_PATH = "WmdTool"                                   # Path to the 'WmdTool' which builds the .WMD file
ORIG_FINAL_DOOM_MODULE_PATH = "ORIG_FDOOM_DOOMSND.WMD"      # Path to the original module file shipped with Final Doom. This script will append new sequences to that file.
OUTPUT_WMD_PATH = "../DOOMSND.WMD"                          # Path to the ouput WMD file file

# This is a list of sequences to add into the output .WMD file on top of all the original Final Doom sequences.
# The first value is the sequence index and the second value is the module file to source the sequence from.
sequenceList = [
    [ 0, "module-sfx_vilsit.json" ],    # sfx_vilsit (Arch-vile sight)
    [ 0, "module-sfx_vipain.json" ],    # sfx_vipain (Arch-vile pain)
    [ 0, "module-sfx_vildth.json" ],    # sfx_vildth (Arch-vile death)
    [ 0, "module-sfx_vilact.json" ],    # sfx_vilact (Arch-vile idle)
    [ 0, "module-sfx_vilatk.json" ],    # sfx_vilatk (Arch-vile attack)
    [ 0, "module-sfx_flamst.json" ],    # sfx_flamst (Arch-vile flame start sound)
    [ 0, "module-sfx_flame.json"  ],    # sfx_flame  (Arch-vile flame burn sound)
    [ 0, "module-sfx_sssit.json"  ],    # sfx_sssit  (Wolf-SS sight)
    [ 0, "module-sfx_ssdth.json"  ],    # sfx_ssdth  (Wolf-SS death)
    [ 0, "module-sfx_keenpn.json" ],    # sfx_keenpn (Commander Keen pain)
    [ 0, "module-sfx_keendt.json" ],    # sfx_keendt (Commander Keen death)
    [ 0, "module-sfx_bossit.json" ],    # sfx_bossit (Icon Of Sin sight)
    [ 0, "module-sfx_bospit.json" ],    # sfx_bospit (Icon Of Sin spawner cube spit)
    [ 0, "module-sfx_bospn.json"  ],    # sfx_bospn  (Icon Of Sin pain)
    [ 0, "module-sfx_bosdth.json" ],    # sfx_bosdth (Icon Of Sin death)
    [ 0, "module-sfx_boscub.json" ],    # sfx_boscub (Icon Of Sin spawner cube spit, additional sound)
]

def copy_sequence_to_wmd(
    src_sequence_index,
    src_wmd_file_path,
    dst_wmd_file_path
):
    # Invoke the WMD tool to add this sequence to the output WMD
    result = subprocess.call(
        [
            WMD_TOOL_PATH,
            "-copy-sequences",
            src_wmd_file_path,
            dst_wmd_file_path,
            str(src_sequence_index)
        ]
    )

    if result != 0:
        print("FAILED to add a sequence to the WMD file!")
        sys.exit(1)


def main():
    # Copy the original Final Doom WMD file to the output location first
    shutil.copyfile(ORIG_FINAL_DOOM_MODULE_PATH, OUTPUT_WMD_PATH)

    # Append all of the new sequences to the new WMD file
    for sequence in sequenceList:
        copy_sequence_to_wmd(sequence[0], sequence[1], OUTPUT_WMD_PATH)

main()
