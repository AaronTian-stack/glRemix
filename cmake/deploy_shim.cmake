# Deploy script for shim
# Usage: cmake -Dbuild_dir=<path> -Dconfig=<config> -Doutput_dir=<path> -P deploy_shim.cmake

if(NOT DEFINED build_dir)
    message(FATAL_ERROR "build_dir not defined")
endif()

if(NOT DEFINED config)
    message(FATAL_ERROR "config not defined")
endif()

if(NOT DEFINED output_dir)
    message(FATAL_ERROR "output_dir not defined")
endif()

set(shim_dll "${build_dir}/${config}/opengl32.dll")
set(shim_pdb "${build_dir}/${config}/opengl32.pdb")

message(STATUS "Deploying shim from ${build_dir}/${config}")
message(STATUS "  DLL: ${shim_dll}")
message(STATUS "  PDB: ${shim_pdb}")
message(STATUS "  Destination: ${output_dir}")

file(MAKE_DIRECTORY "${output_dir}")

if(EXISTS "${shim_dll}")
    file(COPY_FILE "${shim_dll}" "${output_dir}/opengl32.dll")
    message(STATUS "  Copied DLL")
else()
    message(WARNING "  DLL not found: ${shim_dll}")
endif()

if(EXISTS "${shim_pdb}")
    file(COPY_FILE "${shim_pdb}" "${output_dir}/opengl32.pdb")
    message(STATUS "  Copied PDB")
else()
    message(WARNING "  PDB not found: ${shim_pdb}")
endif()
