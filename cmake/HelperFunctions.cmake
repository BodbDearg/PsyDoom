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
