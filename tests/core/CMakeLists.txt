#-------------------------------------------------------------------------------
# Ensure Dependencies
#-------------------------------------------------------------------------------
if (NOT TARGET serialize::core)
    message( FATAL_ERROR "serialize::core could not be found")
endif()
if (NOT TARGET serialize::testutils)
    message( FATAL_ERROR "serialize::testutils could not be found")
endif()

#-------------------------------------------------------------------------------
# CMAKE CONFIGURATIONS
#-------------------------------------------------------------------------------
# No configuration

#-------------------------------------------------------------------------------
# Add test executable
#-------------------------------------------------------------------------------
file(GLOB_RECURSE tests_src src/*.test.cpp src/*.test.cu)
add_executable(tests_core ${tests_src})
target_link_libraries(tests_core PRIVATE serialize::core serialize::testutils Catch2::Catch2WithMain)
target_include_directories(tests_core PRIVATE include/)
catch_discover_tests(tests_core)