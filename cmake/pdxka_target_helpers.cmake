cmake_minimum_required(VERSION 3.21)

##
# pdxka_target_helpers.cmake
#
# Provides CMake target helper functions/macros.
#

##
# Copy the runtime-required DLLs for a target to the target's output directory.
#
# Arguments:
#   target      Target name
#
function(pdxka_copy_runtime_dlls target)
    # copy dependent DLLs to output directory
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:${target}> $<TARGET_FILE_DIR:${target}>
        COMMENT "Copying dependent DLLs for ${target}"
        COMMAND_EXPAND_LISTS
        VERBATIM
    )
endfunction()
