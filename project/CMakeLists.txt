cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

# Project name
project(MBase)

# Variable declarations
string(TOLOWER ${CMAKE_SYSTEM_NAME} OperatingSystemNameLowerCase)

# Visual Studio settings
if(WIN32)
	#Paralell compilation
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /D \"_CRT_SECURE_NO_WARNINGS\"")
endif(WIN32)

# Root directory
set(ProjectRoot ${MBase_SOURCE_DIR}/../)
get_filename_component(ProjectRootAbsolute ${ProjectRoot} ABSOLUTE)

# Include directory
file(RELATIVE_PATH IncludeDirectoryRelative ${MBase_SOURCE_DIR} ${ProjectRootAbsolute}/include)
get_filename_component(IncludeDirectoryAbsolute ${IncludeDirectoryRelative} ABSOLUTE)

# Library directory
file(RELATIVE_PATH LibraryDirectoryRelative ${MBase_SOURCE_DIR} ${ProjectRootAbsolute}/lib/64-bit)
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
    #string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR}) 	# Gets name of folder only
    #string(REPLACE / \\ SRCGR ${SRCGR}) 						# Ensure that only \\ are used (source_group #expects \\, not /)
    #source_group("${SRCGR}" FILES ${f})
#endforeach()

# Create executable
add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_DIRECTORIES})

# Link libraries to the executable
#target_link_libraries(${CMAKE_PROJECT_NAME} ${SDL2Libraries})