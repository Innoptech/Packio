message(STATUS "Fetch zlib")
Include(FetchContent)
FetchContent_Declare(
        zlib
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG        ${zlib_VERSION}
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "Disable ZLib Examples")
FetchContent_MakeAvailable(zlib)