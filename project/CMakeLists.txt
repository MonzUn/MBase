cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

# Project name
project(MEngine)

# Variable declarations
string(TOLOWER ${CMAKE_SYSTEM_NAME} OperatingSystemNameLowerCase)

# Visual Studio settings
if(WIN32)
	#Paralell compilation
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /D \"_CRT_SECURE_NO_WARNINGS\"")
endif(WIN32)

# Root directory
set(ProjectRoot ${MEngine_SOURCE_DIR}/../)
get_filename_component(ProjectRootAbsolute ${ProjectRoot} ABSOLUTE)

# Include directory
file(RELATIVE_PATH IncludeDirectoryRelative ${MEngine_SOURCE_DIR} ${ProjectRootAbsolute}/include)
get_filename_component(IncludeDirectoryAbsolute ${IncludeDirectoryRelative} ABSOLUTE)

# Library directory
file(RELATIVE_PATH LibraryDirectoryRelative ${MEngine_SOURCE_DIR} ${ProjectRootAbsolute}/lib/x64)
get_filename_component(LibraryDirectoryAbsolute ${LibraryDirectoryRelative} ABSOLUTE)

# Ensure build type set
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

# Set output directory
if(WIN32)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ProjectRoot}/bin/${OperatingSystemNameLowerCase}/)
else()
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ProjectRoot}/bin/${OperatingSystemNameLowerCase}/${CMAKE_BUILD_TYPE})
endif(WIN32)
get_filename_component(OutputDirectoryAbsolute ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ABSOLUTE)

# Find SDL2
find_path(SDL2Include SDL2/SDL.h PATHS ${IncludeDirectoryAbsolute})
set(SDL2Includes ${SDL2Include})

set(SDL2LibraryDirectoryAbsolute "${LibraryDirectoryAbsolute}/SDL2")
find_library(SDL2Library
	NAMES SDL2 SDL2main
	PATHS ${SDL2LibraryDirectoryAbsolute})
set(SDL2Libraries ${SDL2Library})

if (WIN32)
	find_library(SDL2mainLibrary
		NAMES SDL2main
		PATHS ${SDL2LibraryDirectoryAbsolute})
	list(APPEND SDL2Libraries ${SDL2mainLibrary})
endif(WIN32)

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

# Generate Visual Studio filters from folder names
#set(RELPATH ${ProjectRoot}/source/)
#get_filename_component(RELPATH ${RELPATH} ABSOLUTE)
#foreach(f ${SOURCE_DIRECTORIES})
    #file(RELATIVE_PATH SRCGR ${RELPATH} ${f})
    #string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR}) # Gets name of folder only
    #string(REPLACE / \\ SRCGR ${SRCGR}) 						# Ensure that only \\ are used (source_group #expects \\, not /)
    #source_group("${SRCGR}" FILES ${f})
#endforeach()

# Create executable
add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_DIRECTORIES})

# Link libraries to the executable
target_link_libraries(${CMAKE_PROJECT_NAME} ${SDL2Libraries})

# Add postbuild scripts that moves DLL files to the directory of the executable
add_custom_command(TARGET ${CMAKE_PROJECT_NAME}
	POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ProjectRootAbsolute}/external/windows/$<CONFIGURATION>" 
        ${OutputDirectoryAbsolute}/$<CONFIGURATION>)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME}
	POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ProjectRootAbsolute}/external/windows/shared" 
        ${OutputDirectoryAbsolute}/$<CONFIGURATION>)