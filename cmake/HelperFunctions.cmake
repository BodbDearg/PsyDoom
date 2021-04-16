#-----------------------------------------------------------------------------------------------------------------------
# Setup grouping/folders for the current target.
# Useful to group source and other files in IDEs like Visual Studio.
#-----------------------------------------------------------------------------------------------------------------------
function(setup_source_groups SOURCE_FILES OTHER_FILES)
    # CMakeLists.txt is special - put it at the root of the project for easy access
    set(OTHER_FILES_MINUS_CMAKE_LISTS "${OTHER_FILES}")
    list(REMOVE_ITEM OTHER_FILES_MINUS_CMAKE_LISTS "CMakeLists.txt")
    source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" PREFIX "" FILES "CMakeLists.txt")

    # Regular source and other files
    if (SOURCE_FILES)
        source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" PREFIX "Source Files" FILES ${SOURCE_FILES})
    endif()

    if (OTHER_FILES_MINUS_CMAKE_LISTS)
        source_group(TREE "${CMAKE_CURRENT_LIST_DIR}" PREFIX "Other Files" FILES ${OTHER_FILES_MINUS_CMAKE_LISTS})
    endif()
endfunction()

#-----------------------------------------------------------------------------------------------------------------------
# Add common target compile options for all projects
#-----------------------------------------------------------------------------------------------------------------------
function(add_common_target_compile_options TARGET_NAME)
    # Enable a very strict level of warnings
    if (COMPILER_MSVC)    
        target_compile_options(${TARGET_NAME} PRIVATE /W4)                  # Enable all warnings
    elseif(COMPILER_GCC OR COMPILER_CLANG)
        target_compile_options(${TARGET_NAME} PRIVATE -pedantic -Wall)      # Enable all warnings and non standard C++ warnings
    endif()

    # MSVC: Don't complain about using regular 'std::fopen()' etc.
    if (COMPILER_MSVC)
        target_compile_definitions(${TARGET_NAME} PRIVATE -D_CRT_SECURE_NO_WARNINGS)
    endif()
endfunction()
