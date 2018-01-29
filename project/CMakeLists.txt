cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

# Project name
project(MEngine CXX)

# OS Name
string(TOLOWER ${CMAKE_SYSTEM_NAME} OperatingSystemNameLowerCase)

# Root directory
set(ProjectRoot ${MEngine_SOURCE_DIR}/../)
get_filename_component(ProjectRootAbsolute ${ProjectRoot} ABSOLUTE)

# Visual Studio settings
if(MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /D \"_CRT_SECURE_NO_WARNINGS\"")
endif(MSVC)

# Include directory
file(RELATIVE_PATH IncludeDirectoryRelative ${MEngine_SOURCE_DIR} ${ProjectRootAbsolute}/include)
get_filename_component(IncludeDirectoryAbsolute ${IncludeDirectoryRelative} ABSOLUTE)

# Debug Library directory
file(RELATIVE_PATH DebugLibraryDirectoryRelative ${MEngine_SOURCE_DIR} ${ProjectRootAbsolute}/lib/debug/x64)
get_filename_component(DebugLibraryDirectoryAbsolute ${DebugLibraryDirectoryRelative} ABSOLUTE)

# Release Library directory
file(RELATIVE_PATH ReleaseLibraryDirectoryRelative ${MEngine_SOURCE_DIR} ${ProjectRootAbsolute}/lib/release/x64)
get_filename_component(ReleaseLibraryDirectoryAbsolute ${ReleaseLibraryDirectoryRelative} ABSOLUTE)

# Shared Library directory
file(RELATIVE_PATH SharedLibraryDirectoryRelative ${MEngine_SOURCE_DIR} ${ProjectRootAbsolute}/lib/shared/x64)
get_filename_component(SharedLibraryDirectoryAbsolute ${SharedLibraryDirectoryRelative} ABSOLUTE)

# Set output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${ProjectRoot}/output/)

# Find SDL2
find_path(SDL2Include SDL2/SDL.h PATHS ${IncludeDirectoryAbsolute})
set(SDL2Includes ${SDL2Include}/SDL2)

# Find MUtility
find_path(MUtilityInclude MUtility/MUtilityByte.h PATHS ${IncludeDirectoryAbsolute})
set(MUtilityIncludes ${MUtilityInclude}/MUtility)
set(MUtilityDebugLibraryDirectoryAbsolute "${DebugLibraryDirectoryAbsolute}/MUtility")
set(MUtilityReleaseLibraryDirectoryAbsolute "${ReleaseLibraryDirectoryAbsolute}/MUtility")

include_directories(
	${SDL2Includes}
	${MUtilityIncludes}
)

# List source file directories
file(GLOB_RECURSE SOURCE_DIRECTORIES
	"../source/*.h"
	"../source/*.cpp"
)

# Create static library
ADD_LIBRARY( ${CMAKE_PROJECT_NAME} STATIC
   	${SOURCE_DIRECTORIES}
)

# Make sure that the linker language gets set to C++
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# Get local(non versioned) paths to dependencies
include("CMakeLocalDependenciesPaths.txt" OPTIONAL)

# Add prebuilds scripts that copies local dependencies to their place within the project
if(MUtilityIncludeCopyPath)
	add_custom_command(TARGET ${CMAKE_PROJECT_NAME}
	PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${MUtilityIncludeCopyPath}
        ${MUtilityIncludes})
endif(MUtilityIncludeCopyPath)

if(MUtilityDebugLibCopyPath)
	add_custom_command(TARGET ${CMAKE_PROJECT_NAME}
	PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${MUtilityDebugLibCopyPath}
        ${MUtilityDebugLibraryDirectoryAbsolute})
endif(MUtilityDebugLibCopyPath)

if(MUtilityReleaseLibCopyPath)
	add_custom_command(TARGET ${CMAKE_PROJECT_NAME}
	PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${MUtilityReleaseLibCopyPath}
        ${MUtilityReleaseLibraryDirectoryAbsolute})
endif(MUtilityReleaseLibCopyPath)

# --- DEBUG COPY PASTE ---
#message("|||--- TEST ---||| ${var}")