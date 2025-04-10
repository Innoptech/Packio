cmake_minimum_required(VERSION 3.22.1)

#-------------------------------------------------------------------------------
# Project Definitions
#-------------------------------------------------------------------------------
project(Packio
        DESCRIPTION "A simple serializer/deserializer utility"
        LANGUAGES CXX)

#-------------------------------------------------------------------------------
# VERSIONING
#-------------------------------------------------------------------------------
add_subdirectory(cmake)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(PYPROJECT_PATH "${CMAKE_CURRENT_BINARY_DIR}/pyproject.toml")
configure_file("${PROJECT_SOURCE_DIR}/pyproject.toml" ${PYPROJECT_PATH})
ReadVersion(${PYPROJECT_PATH})

#-------------------------------------------------------------------------------
# COMPILATION
#-------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 ") #-fno-math-errno do not use -march=native with CUDA!(?)
set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

# Do not allow to build in main repo
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file).
    Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()

# Set the default build type
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
            STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "Debug" "Release")
endif()

#-------------------------------------------------------------------------------
# CMAKE OPTIONS
#-------------------------------------------------------------------------------
option(PACKIO_BUILD_TESTS "Enable the compilation of the test files." OFF)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
endif()

#-------------------------------------------------------------------------------
# Add external components
#-------------------------------------------------------------------------------
ReadDependencyVersion(zstd ${PYPROJECT_PATH})
add_subdirectory(extern/zstd)

#-------------------------------------------------------------------------------
# Add components & Package in simpler header files
#-------------------------------------------------------------------------------
set(PACKIO_MODULES core)
foreach(module ${PACKIO_MODULES})
    add_subdirectory(modules/${module})
endforeach()

#-------------------------------------------------------------------------------
# Tests
#-------------------------------------------------------------------------------
if(PACKIO_BUILD_TESTS)
    ReadDependencyVersion(catch2 ${PYPROJECT_PATH})
    add_subdirectory(extern/catch2)
    # Configure automatic test registration
    list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
    include(CTest)
    include(Catch)
    add_subdirectory(tests)
endif()