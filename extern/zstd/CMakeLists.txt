# Fetch and build Zstd
message(STATUS "Fetching Zstd...")
include(FetchContent)
FetchContent_Declare(
        zstd
        GIT_REPOSITORY https://github.com/facebook/zstd.git
        GIT_TAG        ${zstd_VERSION}
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        SOURCE_SUBDIR build/cmake
)

# If you want to override some Zstd build options *before* populating:
set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "Disable building zstd CLI program")
set(ZSTD_BUILD_TESTS OFF CACHE BOOL "Disable building zstd tests")
set(ZSTD_BUILD_SHARED ON CACHE BOOL "Build shared libzstd")
set(ZSTD_BUILD_STATIC ON CACHE BOOL "Build static libzstd") 

# Fetch and make Zstd available
FetchContent_MakeAvailable(zstd)
