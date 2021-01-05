#!python

############################################################################################################################################
# This script compiles all of the Vulkan GLSL format shaders for the project to SPIR-V binary code.
# The SPIR-V generated is saved in the format of C header files which can then be embedded in the application.
#
# Requirements:
#   (1) The Vulkan SDK 'glslangvalidator' tool must be invokable.
#       Install the SDK and ensure this tool is on your current system's BIN path.
#   (2) This script must be executed from the shaders directory.
############################################################################################################################################
import os
import subprocess
import sys

# Job-specs for all the files to compile.
# Corresponds to the 3 arguments of 'compile_shader'
files_to_compile = [
    [ "colored.vert", "compiled/SPIRV_colored_vert.h", "gSPIRV_colored_vert" ],
    [ "colored.frag", "compiled/SPIRV_colored_frag.h", "gSPIRV_colored_frag" ],
]

# Compiles the input GLSL file to an output .h file with the specified name.
# The C-Array for the shader is given the specified name.
def compile_shader(input_glsl_file, output_c_file, c_var_name):
    result = subprocess.call(
        ["glslangvalidator", "--vn", c_var_name, "-V", "-x", "-o", output_c_file, input_glsl_file]
    )

    if result != 0:
        print("Compile FAILED for file: {0:s}!".format(input_glsl_file))
        sys.exit(1)

# Main script logic: compiles all of the shaders
def main():
    for job_spec in files_to_compile:
        compile_shader(job_spec[0], job_spec[1], job_spec[2])

main()
