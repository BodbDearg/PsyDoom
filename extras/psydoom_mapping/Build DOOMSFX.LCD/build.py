#!python

############################################################################################################################################
# This script builds 'DOOMSFX.LCD' which is an archive containing the sound samples for all general effects (SFX) in PsyDoom.
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
OUTPUT_LCD_PATH = "../DOOMSFX.LCD"      # The LCD file that is built
MODULE_FILE_PATH = "../DOOMSND.WMD"     # The Williams Module (.WMD) file containing all of the sequences and instruments for the game

# All of the VAG sounds to be included in the built DOOMSFX.LCD file.
# The first value of the pair is the patch sample index (in the .WMD file) and the second is the VAG file itself.
soundList = [
    # Original PSX sounds
    [  0, "SAMP0000.vag" ],     # Pistol shot
    [  1, "SAMP0001.vag" ],     # Shotgun shot
    [  2, "SAMP0002.vag" ],     # Entering level
    [  3, "SAMP0003.vag" ],     # Plasma rifle shot
    [  4, "SAMP0004.vag" ],     # BFG9000 charge and shot
    [  5, "SAMP0005.vag" ],     # Chainsaw selection
    [  6, "SAMP0006.vag" ],     # Chainsaw idle
    [  7, "SAMP0007.vag" ],     # Chainsaw connecting
    [  8, "SAMP0008.vag" ],     # Chainsaw use
    [  9, "SAMP0009.vag" ],     # Rocket launcher shot
    [ 10, "SAMP0010.vag" ],     # Deathmatch item respawn
    [ 11, "SAMP0011.vag" ],     # BFG9000 projectile explosion
    [ 12, "SAMP0012.vag" ],     # Generic hellspawn fireball
    [ 13, "SAMP0013.vag" ],     # Generic projectile explosion
    [ 14, "SAMP0014.vag" ],     # Lift/platform start sound
    [ 15, "SAMP0015.vag" ],     # Lift/platform stop sound
    [ 16, "SAMP0016.vag" ],     # Door raise
    [ 17, "SAMP0017.vag" ],     # Door lower
    [ 18, "SAMP0018.vag" ],     # Crusher movement
    [ 19, "SAMP0019.vag" ],     # Switch use
    [ 20, "SAMP0020.vag" ],     # Menu closing sound
    [ 25, "SAMP0025.vag" ],     # Item pickup
    [ 26, "SAMP0026.vag" ],     # Wallhump
    [ 27, "SAMP0027.vag" ],     # Teleporter
    [ 56, "SAMP0056.vag" ],     # Barrel and rocket explosions
    [ 57, "SAMP0057.vag" ],     # Fist punch
    [ 60, "SAMP0060.vag" ],     # Super shotgun shot
    [ 61, "SAMP0061.vag" ],     # Super shotgun breaking open
    [ 62, "SAMP0062.vag" ],     # Super shotgun reloading
    [ 63, "SAMP0063.vag" ],     # Super shotgun closing
    [ 83, "SAMP0083.vag" ],     # Blazing door raise
    [ 84, "SAMP0084.vag" ],     # Blazing door lower
    [ 85, "SAMP0085.vag" ],     # Powerup pickup
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
