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

# Library directory
file(RELATIVE_PATH LibraryDirectoryRelative ${MEngine_SOURCE_DIR} ${ProjectRootAbsolute}/lib/x64)
get_filename_component(LibraryDirectoryAbsolute ${LibraryDirectoryRelative} ABSOLUTE)

# Set output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${ProjectRoot}/output/)

# Ensure build type set
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

# Find SDL2
find_path(SDL2Include SDL2/SDL.h PATHS ${IncludeDirectoryAbsolute})
set(SDL2Includes ${SDL2Include}/SDL2)

set(SDL2LibraryDirectoryAbsolute "${LibraryDirectoryAbsolute}/SDL2")
find_library(SDL2Library
	NAMES SDL2 SDL2main
	PATHS ${SDL2LibraryDirectoryAbsolute})
set(SDL2Libraries ${SDL2Library})

find_library(SDL2mainLibrary
	NAMES SDL2main
	PATHS ${SDL2LibraryDirectoryAbsolute})
list(APPEND SDL2Libraries ${SDL2mainLibrary})

# Set lib and include directories
link_directories(
	"${LibraryDirectoryRelative}/SDL2"
)

include_directories(
	${SDL2Includes}
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

# Link libraries to the output lib
target_link_libraries(${CMAKE_PROJECT_NAME} ${SDL2Libraries})



# --- DEBUG COPY PASTE ---
#message("|||--- TEST ---||| ${var}")