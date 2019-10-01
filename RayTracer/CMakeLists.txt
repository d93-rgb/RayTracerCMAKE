cmake_minimum_required (VERSION 3.9)
cmake_policy(SET CMP0048 NEW)
project (RayTracer VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
# create compiler specific information needed for linting
# with vim plugin YouCompleteMe
set( CMAKE_EXPORT_COMPILE_COMMANDS OFF )

if(WIN32)
	#set(CMAKE_WIN32_EXECUTABLE)
	#LINK_DIRECTORIES("C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x64")
	set(SHCORE_LIB shcore.lib)
endif()

if(NOT WIN32)
    # optimization for g++
    add_definitions("-Ofast")
endif()

# external dependencies
###########################################################################
# glog
set(WITH_GFLAGS OFF CACHE BOOL "Use gflags")
set(BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
#IF(WIN32)
#  ADD_DEFINITIONS( -D GOOGLE_GLOG_DLL_DECL= )
#ENDIF()
add_subdirectory(ext/glog)
#SET_PROPERTY(TARGET glog logging_unittest demangle_unittest utilities_unittest stl_logging_unittest PROPERTY FOLDER "ext")
include_directories (
  ext/glog/src
  ${CMAKE_BINARY_DIR}/ext/glog
)

find_package(OpenMP REQUIRED)
#message(STATUS ${OpenMP_C_FLAGS}) # Link against it for C++)

###########################################################################
# assimp
# turn off some unnecessary options for this ray tracer
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL " " FORCE)
set(ASSIMP_BUILD_BLEND_IMPORTER ON CACHE BOOL " " FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL " " FORCE)
set(ASSIMP_BUILD_OBJ_EXPORTER ON CACHE BOOL " " FORCE)

# disable testing
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL " " FORCE)
# disable VS pdb file generation
set(ASSIMP_INSTALL PDB OFF CACHE BOOL " " FORCE)
# build seperate zlib
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL " " FORCE)

add_subdirectory(ext/assimp)
include_directories (
  ext/assimp/include
  ${CMAKE_BINARY_DIR}/ext/assimp/include
)

###########################################################################
# glm
# disable testing
set(GLM_TEST_ENABLE OFF CACHE BOOL " " FORCE)
add_subdirectory(ext/glm)
include_directories (
  ext/glm
  ${CMAKE_BINARY_DIR}/ext/glm
)

###########################################################################
# AntTweakBar
if(NOT WIN32)
    MESSAGE(STATUS "Building AntTweakBar")
	add_custom_target( 	ANTTWEAKBAR_MAKE
						COMMAND make 
						WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ext/AntTweakBar/src)
endif()

# save all assembled libraries  here
#TODO: Change assimp library contents
set(ALL_RT_LIBRARIES
	glog
	assimp
	${SHCORE_LIB})
link_directories( ${CMAKE_BINARY_DIR}/ext/glm 
    ${CMAKE_BINARY_DIR}/ext/assimp/code
    ${CMAKE_BINARY_DIR}/ext/glog)

# ray tracer source and headers
set (RT_MAIN src/main.cpp)

file (GLOB RT_SRC LIST_DIRECTORIES false
src/camera/*
src/core/*
src/gui/*
src/interaction/*
src/light/*
src/material/*
src/misc/*
src/scene/*
src/shape/*
src/shape/quadric/*
src/texture/*
src/samplers/*
src/image/*
src/threads/*)

file (GLOB RT_HEADERS LIST_DIRECTORIES false
include/camera/*
include/core/*
include/gui/*
include/interaction/*
include/light/*
include/material/*
include/misc/*
include/scene/*
include/shape/*
include/shape/quadric/*
include/texture/*
include/samplers/*
include/image/*
include/threads/*)

include_directories( "include/" )

# Visual Studio filters inside the solution explorer
if(MSVC)
    #SET(OPTIONS WIN32)
	MESSAGE(STATUS "Detected Visual Studio as IDE")
	# source files
	SOURCE_GROUP ("Source Files\\camera" REGULAR_EXPRESSION src/camera/.*)
	SOURCE_GROUP ("Source Files\\core" REGULAR_EXPRESSION src/core/.*)
	SOURCE_GROUP ("Source Files\\gui" REGULAR_EXPRESSION src/gui/.*)
	SOURCE_GROUP ("Source Files\\interaction" REGULAR_EXPRESSION src/interaction/.*)
	SOURCE_GROUP ("Source Files\\light" REGULAR_EXPRESSION src/light/.*)
	SOURCE_GROUP ("Source Files\\material" REGULAR_EXPRESSION src/material/.*)
	SOURCE_GROUP ("Source Files\\misc" REGULAR_EXPRESSION src/misc/.*)
	SOURCE_GROUP ("Source Files\\scene" REGULAR_EXPRESSION src/scene/.*)
	SOURCE_GROUP ("Source Files\\shape" REGULAR_EXPRESSION src/shape/[^/]*cpp)
	SOURCE_GROUP ("Source Files\\shape\\quadric" REGULAR_EXPRESSION src/shape/quadric/.*)
	SOURCE_GROUP ("Source Files\\texture" REGULAR_EXPRESSION src/texture/.*)
	SOURCE_GROUP ("Source Files\\samplers" REGULAR_EXPRESSION src/samplers/.*)
	SOURCE_GROUP ("Source Files\\image" REGULAR_EXPRESSION src/image/.*)


	# header files
	SOURCE_GROUP ("Header Files\\camera" REGULAR_EXPRESSION include/camera/.*)
	SOURCE_GROUP ("Header Files\\core" REGULAR_EXPRESSION include/core/.*)
	SOURCE_GROUP ("Header Files\\gui" REGULAR_EXPRESSION include/gui/.*)
	SOURCE_GROUP ("Header Files\\interaction" REGULAR_EXPRESSION include/interaction/.*)
	SOURCE_GROUP ("Header Files\\light" REGULAR_EXPRESSION include/light/.*)
	SOURCE_GROUP ("Header Files\\material" REGULAR_EXPRESSION include/material/.*)
	SOURCE_GROUP ("Header Files\\misc" REGULAR_EXPRESSION include/misc/.*)
	SOURCE_GROUP ("Header Files\\scene" REGULAR_EXPRESSION include/scene/.*)
	SOURCE_GROUP ("Header Files\\shape" REGULAR_EXPRESSION include/shape/[^/]*h)
	SOURCE_GROUP ("Header Files\\shape\\quadric" REGULAR_EXPRESSION include/shape/quadric/.*)
	SOURCE_GROUP ("Header Files\\texture" REGULAR_EXPRESSION include/texture/.*)
	SOURCE_GROUP ("Header Files\\samplers" REGULAR_EXPRESSION include/samplers/.*)
	SOURCE_GROUP ("Source Files\\image" REGULAR_EXPRESSION src/image/.*)



else(MSVC)
	SOURCE_GROUP (camera REGULAR_EXPRESSION src/camera/.*)
	SOURCE_GROUP (core REGULAR_EXPRESSION src/core/.*)
	SOURCE_GROUP (gui REGULAR_EXPRESSION src/gui/.*)
	SOURCE_GROUP (interaction REGULAR_EXPRESSION src/interaction/.*)
	SOURCE_GROUP (light REGULAR_EXPRESSION src/light/.*)
	SOURCE_GROUP (material REGULAR_EXPRESSION src/material/.*)
	SOURCE_GROUP (misc REGULAR_EXPRESSION src/misc/.*)
	SOURCE_GROUP (scene REGULAR_EXPRESSION src/scene/.*)
	SOURCE_GROUP (shape REGULAR_EXPRESSION src/shape/[^/]*cpp)
	SOURCE_GROUP ("shape/quadric" REGULAR_EXPRESSION src/shape/quadric/.*)
	SOURCE_GROUP (texture REGULAR_EXPRESSION src/texture/.*)
	SOURCE_GROUP (samplers REGULAR_EXPRESSION src/samplers/.*)
	SOURCE_GROUP (image REGULAR_EXPRESSION src/image/.*)

endif()

# add headers to the target for now, because VS does not recognize header files otherwise
# to be inside filters
add_executable( rt ${OPTIONS} ${RT_MAIN} ${RT_SRC} ${RT_HEADERS})

if( NOT WIN32 )
	add_dependencies(rt ANTTWEAKBAR_MAKE)
endif()

target_link_libraries( rt ${ALL_RT_LIBRARIES} OpenMP::OpenMP_CXX)

if(WIN32)
	# post build command for copying dll files
	set (ANTTWEAKBAR_DLL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ext/AntTweakBar/lib" )
	add_custom_command(TARGET rt POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${ANTTWEAKBAR_DLL_DIR}/AntTweakBar64.dll"
			$<TARGET_FILE_DIR:rt>)
endif()
