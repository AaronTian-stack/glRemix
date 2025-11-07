cmake_minimum_required(VERSION 3.18)
project(glRemix-Workspace)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT WIN32)
	message(FATAL_ERROR "glRemix is Windows only")
endif()

# Set repo root for all subprojects
set(REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(GLREMIX_DEPLOY_DIR "${REPO_ROOT}/deploy" CACHE PATH "Directory where output is copied after build")

include(ExternalProject)

option(GLREMIX_BUILD_SHIM_WIN32 "Build 32-bit OpenGL shim" ON)
option(GLREMIX_BUILD_SHIM_X64 "Build 64-bit OpenGL shim" OFF)

set(GLREMIX_COPY_IF_EXISTS_SCRIPT "${REPO_ROOT}/cmake/copy_if_exists.cmake")

set(GLREMIX_SHIM_TARGET_PREFIX "glRemix_shim_")
set(GLREMIX_SHIM_SOURCE_DIR "${REPO_ROOT}/glRemixShim")
set(GLREMIX_SHARED_DIR "${REPO_ROOT}/shared") # Do not delete this

include("${REPO_ROOT}/cmake/shared_files.cmake")
include("${GLREMIX_SHIM_SOURCE_DIR}/cmake/shim_sources.cmake")

set(GLREMIX_SHIM_EXTERNAL_SUFFIX "_builder")

# Add external project builders for the shim config, this is to allow building both 32-bit and 64-bit with the 64-bit renderer
function(glremix_add_shim_builder suffix generator_platform description)
	set(target "${GLREMIX_SHIM_TARGET_PREFIX}${suffix}")
	set(builder "${target}${GLREMIX_SHIM_EXTERNAL_SUFFIX}")
	set(build_dir "${CMAKE_BINARY_DIR}/external/shim-${suffix}")

	ExternalProject_Add(${builder}
		SOURCE_DIR "${GLREMIX_SHIM_SOURCE_DIR}"
		BINARY_DIR "${build_dir}"
		CMAKE_GENERATOR "${CMAKE_GENERATOR}"
		CMAKE_GENERATOR_PLATFORM "${generator_platform}"
		CMAKE_ARGS
			-DCMAKE_CXX_STANDARD=20
			-DCMAKE_CXX_STANDARD_REQUIRED=ON
			-DREPO_ROOT=${REPO_ROOT}
			-DGLREMIX_DEPLOY_DIR=${GLREMIX_DEPLOY_DIR}
			-DGLREMIX_SHIM_DEPLOY_SUBDIR=shim/${suffix}
		BUILD_COMMAND ${CMAKE_COMMAND} --build . --config $<CONFIG>
		INSTALL_COMMAND ""
		BUILD_BYPRODUCTS 
			"${GLREMIX_DEPLOY_DIR}/$<CONFIG>/shim/${suffix}/opengl32.dll"
			"${GLREMIX_DEPLOY_DIR}/$<CONFIG>/shim/${suffix}/opengl32.pdb"
	)
	
	# Add a step to force rebuild check when source files change
	ExternalProject_Add_Step(${builder} force_build_check
		COMMAND ${CMAKE_COMMAND} -E echo "Checking if ${target} needs rebuild"
		DEPENDS ${GLREMIX_SHIM_ALL_FILES}
		DEPENDERS build
		ALWAYS FALSE
	)

	if(GLREMIX_DEPLOY_DIR)
		ExternalProject_Add_Step(${builder} deploy
			COMMAND ${CMAKE_COMMAND}
				-Dbuild_dir=${build_dir}
				-Dconfig=$<CONFIG>
				-Doutput_dir=${GLREMIX_DEPLOY_DIR}/$<CONFIG>/shim/${suffix}
				-P ${REPO_ROOT}/cmake/deploy_shim.cmake
			DEPENDEES build
			ALWAYS 1 # Can't figure out how to make this conditional on the build step running
		)
	endif()

	add_custom_target(${target} ALL
		COMMENT "${description}"
		DEPENDS ${GLREMIX_SHIM_ALL_FILES}
	)

	add_dependencies(${target} ${builder})
	set_property(TARGET ${target} PROPERTY SOURCES ${GLREMIX_SHIM_ALL_FILES})
endfunction()

if((GLREMIX_BUILD_SHIM_WIN32 AND GLREMIX_BUILD_SHIM_X64) OR (NOT GLREMIX_BUILD_SHIM_WIN32 AND NOT GLREMIX_BUILD_SHIM_X64))
	message(FATAL_ERROR "Enable exactly one of GLREMIX_BUILD_SHIM_WIN32 or GLREMIX_BUILD_SHIM_X64")
endif()

if(GLREMIX_BUILD_SHIM_WIN32)
	glRemix_add_shim_builder("win32" "Win32" "Build 32-bit OpenGL shim")
endif()

if(GLREMIX_BUILD_SHIM_X64)
	glRemix_add_shim_builder("x64" "x64" "Build 64-bit OpenGL shim")
endif()

add_subdirectory(glRemixRenderer)

include("${REPO_ROOT}/cmake/shim_helper.cmake")