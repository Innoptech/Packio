# Fetch and build Zstd
message(STATUS "Fetching Zstd...")
include(FetchContent)
# Fetch Zstd without SOURCE_SUBDIR
FetchContent_Declare(
        zstd
        GIT_REPOSITORY https://github.com/facebook/zstd.git
        GIT_TAG        ${zstd_VERSION}
)

# Set options before populate
set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "Disable building zstd CLI program")
set(ZSTD_BUILD_TESTS OFF CACHE BOOL "Disable building zstd tests")
set(ZSTD_BUILD_SHARED ON CACHE BOOL "Build shared libzstd")
set(ZSTD_BUILD_STATIC ON CACHE BOOL "Build static libzstd")

FetchContent_GetProperties(zstd)
if(NOT zstd_POPULATED)
    FetchContent_Populate(zstd)
    add_subdirectory(${zstd_SOURCE_DIR}/build/cmake ${zstd_BINARY_DIR})
endif()

