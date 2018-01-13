# required for C++11 features
CMAKE_MINIMUM_REQUIRED(VERSION 3.0.2)

# project name + language used
PROJECT(Partition)

# Compiler Stuff
# ====================================
# required for gtest which uses /MD
if (MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
    ADD_COMPILE_OPTIONS(/MD)  #  -l409
ELSE(MSVC)
    ADD_COMPILE_OPTIONS(-fno-rtti)
    MESSAGE(STATUS "Compilling without rtti -fno-rtti")
ENDIF(MSVC)

IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    MESSAGE(STATUS "Building Debug Version")
ELSE()
    MESSAGE(STATUS "Building Release Version")
    #if (MSVC)
    #    ADD_COMPILE_OPTIONS(/O2)
    #ELSE(MSVC)
    #    ADD_COMPILE_OPTIONS(-O2)
    #ENDIF(MSVC)
ENDIF()

# specify where compiled files must go
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# add cmake FindXXX script to find pre-compiled libraries if any
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

SET(${CXX_STANDARD_REQUIRED} ON)

# Project's Options
# ====================================

OPTION(BUILD_TESTING        "Enable tests"       OFF)
OPTION(USE_LLVM_IR          "Enable LLVM-IR"     OFF)
OPTION(BUILD_EXAMPLES       "Build examples"     OFF)
OPTION(BUILD_BENCHMARK      "Build Benchmarks"   OFF)
OPTION(BUILD_DOCUMENTATION  "Build docs"         OFF)
OPTION(BUILD_DOXYGEN        "Build Doxygen docs" OFF)
OPTION(BUILD_SPHINX         "Build Sphinx docs"  OFF)
OPTION(BUILD_UI             "Build UI"           OFF)

# Binary/pre-compiled Dependencies
# ====================================

FIND_PACKAGE(Git)
FIND_PACKAGE(gtest QUIET)

# Find revision ID and hash of the sourcetree
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})
INCLUDE(cmake/genrev.cmake)

macro(GroupSources curdir)
	file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/${curdir} ${PROJECT_SOURCE_DIR}/${curdir}/*)

    foreach(child ${children})

		if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${curdir}/${child})
			GroupSources(${curdir}/${child})
		else()
			string(REPLACE "/" "\\" groupname ${curdir})

			source_group(${groupname} FILES ${PROJECT_SOURCE_DIR}/${curdir}/${child})
		endif()
    endforeach()
endmacro()

# Subdirectories
# ====================================

# Dependencies to be compiled
ADD_SUBDIRECTORY(dependencies)


# How to compile main lib
# =========================
ADD_SUBDIRECTORY(src)

# Compile tests ?
IF(BUILD_TESTING)
    # ENABLE_TESTING()

    MESSAGE(STATUS "Building tests")
    ADD_SUBDIRECTORY(tests)
ENDIF(BUILD_TESTING)

IF(BUILD_EXAMPLES)
    MESSAGE(STATUS "Building examples")
    ADD_SUBDIRECTORY(examples)
ENDIF(BUILD_EXAMPLES)

IF(BUILD_BENCHMARK)
    MESSAGE(STATUS "Building Benchmarks")
    ADD_SUBDIRECTORY(bench)
ENDIF(BUILD_BENCHMARK)


