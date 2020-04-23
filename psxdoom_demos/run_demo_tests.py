############################################################################################################################################
# A small script that runs predefined sets of demos against PsyDoom in headless mode, verifying each demo result.
# Used for automated testing of the game.
#
# Usage:
#   python run_demo_tests.py <demoset> <psydoom_path> <demos_dir>
############################################################################################################################################
import os
import subprocess
import sys

# These are the lists of demo sets and expected result files
demosets = {
    "doom" : [
        [ "MAP01_01.LMP", "MAP01_01.result.json" ],
        [ "MAP02_01.LMP", "MAP02_01.result.json" ],
        [ "MAP03_01.LMP", "MAP03_01.result.json" ],
        [ "MAP04_01.LMP", "MAP04_01.result.json" ],
        [ "MAP05_01.LMP", "MAP05_01.result.json" ],
        [ "MAP06_01.LMP", "MAP06_01.result.json" ],
        [ "MAP07_01.LMP", "MAP07_01.result.json" ],
        [ "MAP08_01.LMP", "MAP08_01.result.json" ],
        [ "MAP09_01.LMP", "MAP09_01.result.json" ],
        [ "MAP10_01.LMP", "MAP10_01.result.json" ],
        [ "MAP11_01.LMP", "MAP11_01.result.json" ],
        [ "MAP12_01.LMP", "MAP12_01.result.json" ],
        [ "MAP13_01.LMP", "MAP13_01.result.json" ],
        [ "MAP14_01.LMP", "MAP14_01.result.json" ],
        [ "MAP15_01.LMP", "MAP15_01.result.json" ],
        [ "MAP16_01.LMP", "MAP16_01.result.json" ],
        [ "MAP17_01.LMP", "MAP17_01.result.json" ],
        [ "MAP18_01.LMP", "MAP18_01.result.json" ],
        [ "MAP19_01.LMP", "MAP19_01.result.json" ],
        [ "MAP20_01.LMP", "MAP20_01.result.json" ],
        [ "MAP21_01.LMP", "MAP21_01.result.json" ],
        [ "MAP22_01.LMP", "MAP22_01.result.json" ],
        [ "MAP23_01.LMP", "MAP23_01.result.json" ],
        [ "MAP24_01.LMP", "MAP24_01.result.json" ],
        [ "MAP25_01.LMP", "MAP25_01.result.json" ],
        [ "MAP26_01.LMP", "MAP26_01.result.json" ],
        [ "MAP27_01.LMP", "MAP27_01.result.json" ],
        [ "MAP28_01.LMP", "MAP28_01.result.json" ],
        [ "MAP29_01.LMP", "MAP29_01.result.json" ],
        [ "MAP30_01.LMP", "MAP30_01.result.json" ],
        [ "MAP31_01.LMP", "MAP31_01.result.json" ],
        [ "MAP32_01.LMP", "MAP32_01.result.json" ],
        [ "MAP33_01.LMP", "MAP33_01.result.json" ],
        [ "MAP34_01.LMP", "MAP34_01.result.json" ],
        [ "MAP35_01.LMP", "MAP35_01.result.json" ],
        [ "MAP36_01.LMP", "MAP36_01.result.json" ],
        [ "MAP37_01.LMP", "MAP37_01.result.json" ],
        [ "MAP38_01.LMP", "MAP38_01.result.json" ],
        [ "MAP39_01.LMP", "MAP39_01.result.json" ],
        [ "MAP40_01.LMP", "MAP40_01.result.json" ],
        [ "MAP41_01.LMP", "MAP41_01.result.json" ],
        [ "MAP42_01.LMP", "MAP42_01.result.json" ],
        [ "MAP43_01.LMP", "MAP43_01.result.json" ],
        [ "MAP44_01.LMP", "MAP44_01.result.json" ],
        [ "MAP45_01.LMP", "MAP45_01.result.json" ],
        [ "MAP46_01.LMP", "MAP46_01.result.json" ],
        [ "MAP47_01.LMP", "MAP47_01.result.json" ],
        [ "MAP48_01.LMP", "MAP48_01.result.json" ],
        [ "MAP49_01.LMP", "MAP49_01.result.json" ],
        [ "MAP50_01.LMP", "MAP50_01.result.json" ],
        [ "MAP51_01.LMP", "MAP51_01.result.json" ],
        [ "MAP52_01.LMP", "MAP52_01.result.json" ],
        [ "MAP53_01.LMP", "MAP53_01.result.json" ],
        [ "MAP54_01.LMP", "MAP54_01.result.json" ],
        [ "MAP55_01.LMP", "MAP55_01.result.json" ],
        [ "MAP56_01.LMP", "MAP56_01.result.json" ],
        [ "MAP57_01.LMP", "MAP57_01.result.json" ],
        [ "MAP58_01.LMP", "MAP58_01.result.json" ],
        [ "MAP59_01.LMP", "MAP59_01.result.json" ],
    ]
}

# Verify program args
if len(sys.argv) != 4:
    print("Usage: python run_demo_tests.py <demoset> <psydoom_path> <demos_dir>")
    sys.exit(1)

psydoom_path = sys.argv[2]
demos_dir = sys.argv[3]

# Verify demoset is okay
demoset = demosets.get(sys.argv[1])

if not demoset:
    print("Invalid demoset '{0:s}'!".format(sys.argv[1]))
    sys.exit(1)

# Start running the demos and verify they match the expected results
all_tests_passed = True

for demofile_and_result in demoset:
    # Show what demo we are about to run
    demo_path = os.path.join(demos_dir, demofile_and_result[0])
    result_path = os.path.join(demos_dir, demofile_and_result[1])    
    print("Run demo: {0:s}".format(demo_path))
    sys.stdout.flush()

    # Execute the demo using PsyDoom in headless mode and verify the result.
    # PsyDoom will return '0' if the demo was successful.
    result = subprocess.call(
        [psydoom_path, "-headless", "-playdemo", demo_path, "-checkresult", result_path],
        shell=False,
        stdout=subprocess.PIPE,     # Hide output
        stderr=subprocess.PIPE      # Hide output
    )

    # If the test failed (error code != 0) then inform the user
    if result != 0:
        print("    DEMO TEST FAILED!: {0:s}".format(demo_path))
        sys.stdout.flush()
        all_tests_passed = False

# Print the overall result
if all_tests_passed:
    print("All tests executed successfully!")
else:
    print("Some tests FAILED! Overall result is FAIL!")
