message(STATUS "Adding Serialize tests suite")

#-------------------------------------------------------------------------------
# Ensure Dependencies
#-------------------------------------------------------------------------------
if (NOT TARGET Catch2::Catch2WithMain)
    message( FATAL_ERROR "catch2 could not be found")
endif()

#-------------------------------------------------------------------------------
# Add tests
#-------------------------------------------------------------------------------
add_subdirectory(testutils)
add_subdirectory(core)
