# Utility script to copy files

if(NOT DEFINED src)
    return()
endif()

if("${src}" STREQUAL "")
    return()
endif()

if(NOT EXISTS "${src}")
    return()
endif()

if(NOT DEFINED dst)
    message(FATAL_ERROR "dst variable not provided to copy_if_exists script")
endif()

get_filename_component(_copy_dir "${dst}" DIRECTORY)
if(NOT EXISTS "${_copy_dir}")
    file(MAKE_DIRECTORY "${_copy_dir}")
endif()

configure_file("${src}" "${dst}" COPYONLY)
