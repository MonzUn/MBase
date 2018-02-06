cmake_minimum_required(VERSION 3.8.2)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

# Project name
project(MEngine CXX)

# OS Name
string(TOLOWER ${CMAKE_SYSTEM_NAME} OperatingSystemNameLowerCase)

# Root directory
set(ProjectRoot ${CMAKE_CURRENT_LIST_DIR}/../)
get_filename_component(ProjectRootAbsolute ${ProjectRoot} ABSOLUTE)

# Visual Studio settings
if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /D _CRT_SECURE_NO_WARNINGS")
endif(MSVC)

# Include directory
file(RELATIVE_PATH IncludeDirectoryRelative ${MEngine_SOURCE_DIR} ${ProjectRootAbsolute}/include)
get_filename_component(IncludeDirectoryAbsolute ${IncludeDirectoryRelative} ABSOLUTE)

# Externals directory
set(SharedExternalsPathAbsolute "${ProjectRootAbsolute}/external/windows/shared")

# Debug Library directory
file(RELATIVE_PATH DebugLibraryDirectoryRelative ${MEngine_SOURCE_DIR} ${ProjectRootAbsolute}/lib/debug/x64)
get_filename_component(DebugLibraryDirectoryAbsolute ${DebugLibraryDirectoryRelative} ABSOLUTE)

# Release Library directory
file(RELATIVE_PATH ReleaseLibraryDirectoryRelative ${MEngine_SOURCE_DIR} ${ProjectRootAbsolute}/lib/release/x64)
get_filename_component(ReleaseLibraryDirectoryAbsolute ${ReleaseLibraryDirectoryRelative} ABSOLUTE)

# Shared Library directory
file(RELATIVE_PATH SharedLibraryDirectoryRelative ${MEngine_SOURCE_DIR} ${ProjectRootAbsolute}/lib/shared/x64)
get_filename_component(SharedLibraryDirectoryAbsolute ${SharedLibraryDirectoryRelative} ABSOLUTE)

# Find SDL2
set(SDL2Includes ${IncludeDirectoryAbsolute}/SDL2)
set(SDL2LibraryDirectoryAbsolute "${SharedLibraryDirectoryAbsolute}/SDL2")
message("[${PROJECT_NAME}] - Assuming SDL2 includes are located at path ${SDL2Includes}")

# Find MUtility
set(MUtilityIncludes "${IncludeDirectoryAbsolute}/MUtility")
set(MUtilityDebugLibraryDirectoryAbsolute "${DebugLibraryDirectoryAbsolute}/MUtility")
set(MUtilityReleaseLibraryDirectoryAbsolute "${ReleaseLibraryDirectoryAbsolute}/MUtility")
message("[${PROJECT_NAME}] - Assuming MUtility includes are located at path ${MUtilityIncludes}")

# List include directories
set(IncludeDirectoryList
	${SDL2Includes}
	${MUtilityIncludes}
)

# List source file directories
file(GLOB_RECURSE SOURCE_DIRECTORIES
	"../source/*.h"
	"../source/*.cpp"
)

# Create static library
ADD_LIBRARY( ${PROJECT_NAME} STATIC ${SOURCE_DIRECTORIES})

# Set include directories
set_property(TARGET ${PROJECT_NAME} PROPERTY INCLUDE_DIRECTORIES ${IncludeDirectoryList})
	
# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${ProjectRootAbsolute}/output/)

# Make sure that the linker language gets set to C++
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# ---------- Dependencies ----------

# Get local(non versioned) paths to dependencies (Note: These values will be inherited to subdirectories if the subdirectories do not specify their own paths)
include("${CMAKE_CURRENT_LIST_DIR}/CMakeLocalDependenciesPaths.txt" OPTIONAL)

if(MUtilityRootPath)
	message("[${PROJECT_NAME}] - Found MUtility project at path ${MUtilityRootPath}")
	set(MUtilityCMakePath "${MUtilityRootPath}/project")
	set(MUtilityIncludeCopyPath "${MUtilityRootPath}/source/interface")
	set(MUtilityDebugLibCopyPath "${MUtilityRootPath}/output/Debug")
	set(MUtilityReleaseLibCopyPath "${MUtilityRootPath}/output/Release")
	add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${MUtilityIncludeCopyPath} ${MUtilityIncludes})
	add_custom_command(TARGET ${PROJECT_NAME} PRE_LINK COMMAND ${CMAKE_COMMAND} -E copy_directory ${MUtilityDebugLibCopyPath} ${MUtilityDebugLibraryDirectoryAbsolute})
	add_custom_command(TARGET ${PROJECT_NAME} PRE_LINK COMMAND ${CMAKE_COMMAND} -E copy_directory ${MUtilityReleaseLibCopyPath} ${MUtilityReleaseLibraryDirectoryAbsolute})
	if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
		add_subdirectory(${MUtilityCMakePath} ${MUtilityCMakePath})
	endif(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
endif(MUtilityRootPath)

if(SDL2LibPath)
	message("[${PROJECT_NAME}] - Found SDL2lib copy path at ${SDL2LibPath}")
	add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${SDL2LibPath} ${SDL2LibraryDirectoryAbsolute})
endif(SDL2LibPath)

if(SDL2DLLPath)
	message("[${PROJECT_NAME}] - Found SDL2DLL copy path ${SDL2DLLPath}")
	add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${SDL2DLLPath} ${SharedExternalsPathAbsolute})
endif(SDL2DLLPath)

# --- DEBUG COPY PASTE ---
#message("|||--- TEST ---||| ${var}")