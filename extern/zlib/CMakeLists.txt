message(STATUS "Fetch zlib")
Include(FetchContent)
FetchContent_Declare(
        zlib
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG        ${zlib_VERSION}
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(zlib)