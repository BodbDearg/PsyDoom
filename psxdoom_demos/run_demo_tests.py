############################################################################################################################################
# A small script that runs predefined sets of demos against PsyDoom in headless mode, verifying each demo result.
# Used for automated testing of the game.
#
# Usage:
#   python run_demo_tests.py <demoset|all> <psydoom_path> <demos_dir>
############################################################################################################################################
import multiprocessing
import os
import subprocess
import sys
import time

# These are the lists of demo sets and expected result files
demosets = {
    # PSX Doom: NTSC-U or NTSC-J, original or Greatest Hits (1.1)
    "doom_ntsc" : {
        "cue_file" : "Doom_NTSC.cue",
        "tests" : [
            [ "DOOM_NTSC_MAP{0:02}.LMP".format(i), "DOOM_NTSC_MAP{0:02}.result.json".format(i) ] for i in range(1, 60)
        ] + [
            # Original game demos - make sure these work too
            [ "DOOM_NTSC_DEMO1.LMP", "DOOM_NTSC_DEMO1.result.json" ],
            [ "DOOM_NTSC_DEMO2.LMP", "DOOM_NTSC_DEMO2.result.json" ]
        ]
    },
    # PSX Doom: PAL, original or Platinum (1.1)
    "doom_pal" : {
        "cue_file" : "Doom_PAL.cue",
        "tests" :  [
            [ "DOOM_PAL_MAP{0:02}.LMP".format(i), "DOOM_PAL_MAP{0:02}.result.json".format(i) ] for i in range(1, 47)
        ] + [
            # Original game demos - make sure these work too
            [ "DOOM_PAL_DEMO1.LMP", "DOOM_PAL_DEMO1.result.json" ],
            [ "DOOM_PAL_DEMO2.LMP", "DOOM_PAL_DEMO2.result.json" ]
        ]
    },
    # Final Doom: NTSC-U or NTSC-J
    "final_doom_ntsc" : {
        "cue_file" : "FinalDoom_NTSC.cue",
        "tests" : [
            # Gamepad playthrough
            [ "FDOOM_NTSC_PAD_MAP{0:02}.LMP".format(i), "FDOOM_NTSC_PAD_MAP{0:02}.result.json".format(i) ] for i in range(1, 31)
        ] + [
            # Mouse demos - just to verify mouse controls
            [ "FDOOM_NTSC_MOUSE_MAP01.LMP", "FDOOM_NTSC_MOUSE_MAP01.result.json" ],
            [ "FDOOM_NTSC_MOUSE_MAP02.LMP", "FDOOM_NTSC_MOUSE_MAP02.result.json" ],
            [ "FDOOM_NTSC_MOUSE_MAP16.LMP", "FDOOM_NTSC_MOUSE_MAP16.result.json" ],
            [ "FDOOM_NTSC_MOUSE_MAP17.LMP", "FDOOM_NTSC_MOUSE_MAP17.result.json" ],
        ] + [
            # Original game demos - make sure these work too
            [ "FDOOM_NTSC_DEMO1.LMP", "FDOOM_NTSC_DEMO1.result.json" ],
            [ "FDOOM_NTSC_DEMO2.LMP", "FDOOM_NTSC_DEMO2.result.json" ]
        ]
    },
    # Final Doom: PAL
    "final_doom_pal" : {
        "cue_file" : "FinalDoom_PAL.cue",
        "tests" : [
            # Gamepad playthrough
            [ "FDOOM_PAL_PAD_MAP{0:02}.LMP".format(i), "FDOOM_PAL_PAD_MAP{0:02}.result.json".format(i) ] for i in range(1, 31)
        ] + [
            # Mouse demos - just to verify mouse controls
            [ "FDOOM_PAL_MOUSE_MAP01.LMP", "FDOOM_PAL_MOUSE_MAP01.result.json" ],
            [ "FDOOM_PAL_MOUSE_MAP14.LMP", "FDOOM_PAL_MOUSE_MAP14.result.json" ],
        ] + [
            # These demos were problem cases previously
            [ "FDOOM_PAL_PAD_MAP01_2.LMP", "FDOOM_PAL_PAD_MAP01_2.result.json" ],
            [ "FDOOM_PAL_PAD_MAP01_3.LMP", "FDOOM_PAL_PAD_MAP01_3.result.json" ],
        ] + [
            # Original game demos - make sure these work too
            [ "FDOOM_PAL_DEMO1.LMP", "FDOOM_PAL_DEMO1.result.json" ],
            [ "FDOOM_PAL_DEMO2.LMP", "FDOOM_PAL_DEMO2.result.json" ]
        ]
    },
}

# This function executes the demo in a worker process
def run_demo(psydoom_path, cue_file_path, demos_dir, demo_and_result):
    # Show what demo we are about to run
    demo_path = os.path.join(demos_dir, demo_and_result[0])
    result_path = os.path.join(demos_dir, demo_and_result[1])    

    # Execute the demo using PsyDoom in headless mode and verify the result.
    # PsyDoom will return '0' if the demo was successful.
    result = subprocess.call(
        [psydoom_path, "-cue", cue_file_path, "-headless", "-playdemo", demo_path, "-checkresult", result_path],
        shell=False,
        stdout=subprocess.PIPE,     # Hide output
        stderr=subprocess.PIPE      # Hide output
    )

    # If the test failed (error code != 0) then inform the user.
    # Otherwise print that the test succeeded
    if result == 0:
        print("Test passed: {0:s}".format(demo_path))
    else:
        print("[TEST FAIL] Unexpected demo result!: {0:s}".format(demo_path))
        sys.exit(1)

# High level script logic
def main():    
    # Verify program args
    if len(sys.argv) != 4:
        print("Usage: python run_demo_tests.py <demoset|all> <psydoom_path> <demos_dir>")
        sys.exit(1)

    psydoom_path = sys.argv[2]
    demos_dir = sys.argv[3]

    # Verify demoset argument is okay or 'all' is specified
    demoset_arg = sys.argv[1]
    single_demoset = demosets.get(demoset_arg)

    if not single_demoset and demoset_arg != "all":
        print("Invalid demoset '{0:s}'!".format(demoset_arg))
        sys.exit(1)

    if single_demoset:
        run_demosets = [ single_demoset ]
    else:
        run_demosets = demosets.values()

    # Start running the demos and verify they match the expected results
    all_tests_passed = True
    start_time = time.time()
    jobs = []

    for demoset in run_demosets:
        for demo_and_result in demoset["tests"]:
            job = multiprocessing.Process(
                target=run_demo,
                args=(psydoom_path, demoset["cue_file"], demos_dir, demo_and_result)
            )
            job.start()
            jobs.append(job)

    for job in jobs:
        job.join()
        if job.exitcode != 0:
            all_tests_passed = False

    # Print the overall result
    if all_tests_passed:
        print("All tests executed successfully!")
    else:
        print("Some tests FAILED! Overall result is FAIL!")

    # Print time taken
    time_taken = time.time() - start_time
    print("Time taken: {0:f} seconds".format(time_taken))

# This is required for correct parallelism on Windows.
# See: https://stackoverflow.com/questions/18204782/runtimeerror-on-windows-trying-python-multiprocessing
if __name__ == '__main__':
    main()
