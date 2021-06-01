#!python

############################################################################################################################################
# This script builds 'ALLMAPS.LCD' which is an archive containing the sound samples for all possible enemies in PsyDoom.
# This .LCD file can be redistributed with new user maps to provide all required audio for those maps. 
#
# Requirements:
#   (1) The PsyDoom audio tool 'LcdTool' must be invokable on the command line (add it to the system path).
#   (2) This script must be executed from this directory.
############################################################################################################################################
import os
import subprocess
import sys

# Script paths
LCD_TOOL_PATH = "LcdTool"               # Path to the 'LcdTool' which builds the .LCD file
OUTPUT_LCD_PATH = "../ALLMAPS.LCD"      # The LCD file that is built
MODULE_FILE_PATH = "../DOOMSND.WMD"     # The Williams Module (.WMD) file containing all of the sequences and instruments for the game

# All of the VAG sounds to be included in the built ALLMAPS.LCD file.
# The first value of the pair is the patch sample index (in the .WMD file) and the second is the VAG file itself.
soundList = [
    # Original PSX sounds
    [ 12, "SAMP0012.vag" ],     # Fireball throw sound
    [ 13, "SAMP0013.vag" ],     # Fireball explode sound
    [ 21, "SAMP0021.vag" ],     # Player pain
    [ 22, "SAMP0022.vag" ],     # Demon pain
    [ 23, "SAMP0023.vag" ],     # Zombie/imp pain
    [ 24, "SAMP0024.vag" ],     # Squelch
    [ 28, "SAMP0028.vag" ],     # Zombie sight 1
    [ 29, "SAMP0029.vag" ],     # Zombie sight 2
    [ 30, "SAMP0030.vag" ],     # Zombie sight 3 (unused by the retail code)
    [ 31, "SAMP0031.vag" ],     # Imp sight 1
    [ 32, "SAMP0032.vag" ],     # Imp sight 2
    [ 33, "SAMP0033.vag" ],     # Demon sight
    [ 34, "SAMP0034.vag" ],     # Cacodemon sight
    [ 35, "SAMP0035.vag" ],     # Baron Of Hell sight
    [ 36, "SAMP0036.vag" ],     # Cyberdemon sight
    [ 37, "SAMP0037.vag" ],     # Spider Mastermind sight
    [ 38, "SAMP0038.vag" ],     # Lost Soul attack
    [ 39, "SAMP0039.vag" ],     # Demon attack
    [ 40, "SAMP0040.vag" ],     # Imp melee attack
    [ 41, "SAMP0041.vag" ],     # Player die
    [ 42, "SAMP0042.vag" ],     # Zombie die 1
    [ 43, "SAMP0043.vag" ],     # Zombie die 2
    [ 44, "SAMP0044.vag" ],     # Zombie die 3 (unused by the retail code)
    [ 45, "SAMP0045.vag" ],     # Imp die 1
    [ 46, "SAMP0046.vag" ],     # Imp die 2
    [ 47, "SAMP0047.vag" ],     # Demon die
    [ 48, "SAMP0048.vag" ],     # Cacodemon die
    [ 49, "SAMP0049.vag" ],     # Unsure what this is... (unused?)
    [ 50, "SAMP0050.vag" ],     # Baron Of Hell die
    [ 51, "SAMP0051.vag" ],     # Cyberdemon die
    [ 52, "SAMP0052.vag" ],     # Spider Mastermind die
    [ 53, "SAMP0053.vag" ],     # Zombie idle
    [ 54, "SAMP0054.vag" ],     # Imp idle
    [ 55, "SAMP0055.vag" ],     # Demon idle
    [ 56, "SAMP0056.vag" ],     # Rocket blast
    [ 58, "SAMP0058.vag" ],     # Cyberdemon hoof up
    [ 59, "SAMP0059.vag" ],     # Cyberdemon hoof thud
    [ 64, "SAMP0064.vag" ],     # Knight Of Hell sight
    [ 65, "SAMP0065.vag" ],     # Knight Of Hell die
    [ 66, "SAMP0066.vag" ],     # Pain Elemental sight
    [ 67, "SAMP0067.vag" ],     # Pain Elemental pain
    [ 68, "SAMP0068.vag" ],     # Pain Elemental die
    [ 69, "SAMP0069.vag" ],     # Arachnotron sight
    [ 70, "SAMP0070.vag" ],     # Arachnotron die
    [ 71, "SAMP0071.vag" ],     # Arachnotron idle
    [ 72, "SAMP0072.vag" ],     # Arachnotron stomp
    [ 73, "SAMP0073.vag" ],     # Mancubus attack
    [ 74, "SAMP0074.vag" ],     # Mancubus sight
    [ 75, "SAMP0075.vag" ],     # Mancubus pain
    [ 76, "SAMP0076.vag" ],     # Mancubus die
    [ 77, "SAMP0077.vag" ],     # Revenant sight
    [ 78, "SAMP0078.vag" ],     # Revenant die
    [ 79, "SAMP0079.vag" ],     # Revenant idle
    [ 80, "SAMP0080.vag" ],     # Revenant missile attack
    [ 81, "SAMP0081.vag" ],     # Revenant punch
    [ 82, "SAMP0082.vag" ],     # Revenant punch land

    # New sounds for PsyDoom
    [ 144, "sfx_vilsit.vag" ],  # sfx_vilsit (Arch-vile sight)
    [ 145, "sfx_vipain.vag" ],  # sfx_vipain (Arch-vile pain)
    [ 146, "sfx_vildth.vag" ],  # sfx_vildth (Arch-vile death)
    [ 147, "sfx_vilact.vag" ],  # sfx_vilact (Arch-vile idle)
    [ 148, "sfx_vilatk.vag" ],  # sfx_vilatk (Arch-vile attack)
    [ 149, "sfx_flamst.vag" ],  # sfx_flamst (Arch-vile flame start sound)
    [ 150, "sfx_flame.vag"  ],  # sfx_flame  (Arch-vile flame burn sound)
    [ 151, "sfx_sssit.vag"  ],  # sfx_sssit  (Wolf-SS sight)
    [ 152, "sfx_ssdth.vag"  ],  # sfx_ssdth  (Wolf-SS death)
    [ 153, "sfx_keenpn.vag" ],  # sfx_keenpn (Commander Keen pain)
    [ 154, "sfx_keendt.vag" ],  # sfx_keendt (Commander Keen death)
    [ 155, "sfx_bossit.vag" ],  # sfx_bossit (Icon Of Sin sight)
    [ 156, "sfx_bospit.vag" ],  # sfx_bospit (Icon Of Sin spawner cube spit)
    [ 157, "sfx_bospn.vag"  ],  # sfx_bospn  (Icon Of Sin pain)
    [ 158, "sfx_bosdth.vag" ],  # sfx_bosdth (Icon Of Sin death)
    [ 159, "sfx_boscub.vag" ],  # sfx_boscub (Icon Of Sin spawner cube spit, additional sound)
]

def add_file_to_lcd(
    lcd_file_path,
    wmd_file_path,
    patch_sample_index,
    vag_file_path,
    append_to_lcd
):
    # Invoke the LCD tool to add this file to the LCD
    result = subprocess.call(
        [
            LCD_TOOL_PATH,
            lcd_file_path,
            wmd_file_path,
            "-append" if append_to_lcd else "-create",
            str(patch_sample_index),
            vag_file_path
        ]
    )

    if result != 0:
        print("FAILED to add file to LCD: {0:s}!".format(vag_file_path))
        sys.exit(1)


def main():
    append_to_lcd = False

    for sound in soundList:
        add_file_to_lcd(OUTPUT_LCD_PATH, MODULE_FILE_PATH, sound[0], sound[1], append_to_lcd)
        append_to_lcd = True

main()
