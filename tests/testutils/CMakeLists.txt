#-------------------------------------------------------------------------------
# Ensure Dependencies
#-------------------------------------------------------------------------------
if (NOT TARGET packio)
    message( FATAL_ERROR "packio could not be found")
endif()

#-------------------------------------------------------------------------------
# Add test executable
#-------------------------------------------------------------------------------
add_library(tests_utils INTERFACE)
target_link_libraries(tests_utils INTERFACE packio)
target_include_directories(tests_utils INTERFACE include/)
add_library(packio::testutils ALIAS tests_utils)