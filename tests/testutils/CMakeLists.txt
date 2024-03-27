#-------------------------------------------------------------------------------
# Ensure Dependencies
#-------------------------------------------------------------------------------
if (NOT TARGET serialize::core)
    message( FATAL_ERROR "serialize::core could not be found")
endif()

#-------------------------------------------------------------------------------
# Add test executable
#-------------------------------------------------------------------------------
add_library(tests_utils INTERFACE)
target_link_libraries(tests_utils INTERFACE serialize::core)
target_include_directories(tests_utils INTERFACE include/)
add_library(serialize::testutils ALIAS tests_utils)