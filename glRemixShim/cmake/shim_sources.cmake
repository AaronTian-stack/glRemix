# Building Visual Studio file lists requires the top level CMakeLists.txt to know every shim source, so declare them in a shared helper here.

set(GLREMIX_SHIM_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
set(GLREMIX_SHARED_DIR "${GLREMIX_SHIM_SOURCE_DIR}/../shared")

include("${GLREMIX_SHIM_SOURCE_DIR}/../cmake/shared_files.cmake")

set(GLREMIX_SHIM_SOURCE_FILES
    "${GLREMIX_SHIM_SOURCE_DIR}/dllmain.cpp"
    "${GLREMIX_SHIM_SOURCE_DIR}/gl_exports.cpp"
    "${GLREMIX_SHIM_SOURCE_DIR}/gl_loader.cpp"
    "${GLREMIX_SHIM_SOURCE_DIR}/gl_hooks.cpp"
    "${GLREMIX_SHIM_SOURCE_DIR}/wgl_exports.cpp"
    "${GLREMIX_SHIM_SOURCE_DIR}/frame_recorder.cpp"
)

set(GLREMIX_SHIM_HEADER_FILES
    "${GLREMIX_SHIM_SOURCE_DIR}/framework.h"
    "${GLREMIX_SHIM_SOURCE_DIR}/gl_hooks.h"
    "${GLREMIX_SHIM_SOURCE_DIR}/gl_loader.h"
    "${GLREMIX_SHIM_SOURCE_DIR}/wgl_export_aliases.inl"
    "${GLREMIX_SHIM_SOURCE_DIR}/export_macros.h"
    "${GLREMIX_SHIM_SOURCE_DIR}/undef_export_macros.h"
    "${GLREMIX_SHIM_SOURCE_DIR}/frame_recorder.h"
)

set(GLREMIX_SHIM_SCRIPT_FILES
    "${GLREMIX_SHIM_SOURCE_DIR}/scripts/generate_gl_wrappers.py"
    "${GLREMIX_SHIM_SOURCE_DIR}/scripts/generate_wgl_wrappers.py"
)

set(GLREMIX_SHIM_ALL_FILES
    ${GLREMIX_SHIM_SOURCE_FILES}
    ${GLREMIX_SHIM_HEADER_FILES}
    ${GLREMIX_SHIM_SCRIPT_FILES}
    ${GLREMIX_SHARED_HEADER_FILES}
    ${GLREMIX_SHARED_SOURCE_FILES}
)

set(GLREMIX_SHIM_SOURCE_FILES_REL)
foreach(_shim_src IN LISTS GLREMIX_SHIM_SOURCE_FILES)
    file(RELATIVE_PATH _shim_rel "${GLREMIX_SHIM_SOURCE_DIR}" "${_shim_src}")
    list(APPEND GLREMIX_SHIM_SOURCE_FILES_REL "${_shim_rel}")
endforeach()

set(GLREMIX_SHIM_HEADER_FILES_REL)
foreach(_shim_hdr IN LISTS GLREMIX_SHIM_HEADER_FILES)
    file(RELATIVE_PATH _shim_rel "${GLREMIX_SHIM_SOURCE_DIR}" "${_shim_hdr}")
    list(APPEND GLREMIX_SHIM_HEADER_FILES_REL "${_shim_rel}")
endforeach()

set(GLREMIX_SHIM_SCRIPT_FILES_REL)
foreach(_shim_script IN LISTS GLREMIX_SHIM_SCRIPT_FILES)
    file(RELATIVE_PATH _shim_rel "${GLREMIX_SHIM_SOURCE_DIR}" "${_shim_script}")
    list(APPEND GLREMIX_SHIM_SCRIPT_FILES_REL "${_shim_rel}")
endforeach()

glremix_shared_headers_relative(GLREMIX_SHARED_HEADER_FILES_REL "${GLREMIX_SHIM_SOURCE_DIR}")
glremix_shared_sources_relative(GLREMIX_SHARED_SOURCE_FILES_REL "${GLREMIX_SHIM_SOURCE_DIR}")

if(NOT DEFINED GLREMIX_SHIM_SUPPRESS_SOURCE_GROUPS)
    source_group(TREE "${GLREMIX_SHIM_SOURCE_DIR}" FILES ${GLREMIX_SHIM_SOURCE_FILES} ${GLREMIX_SHIM_HEADER_FILES} ${GLREMIX_SHIM_SCRIPT_FILES})
    source_group(TREE "${GLREMIX_SHARED_DIR}" PREFIX "shared" FILES ${GLREMIX_SHARED_HEADER_FILES} ${GLREMIX_SHARED_SOURCE_FILES})
endif()
