message(STATUS "Adding packio module")

#-------------------------------------------------------------------------------
# Ensure requirements
#-------------------------------------------------------------------------------
if (NOT TARGET zlib)
    message( FATAL_ERROR "zlib could not be found")
endif()
#-------------------------------------------------------------------------------
# CMAKE OPTIONS
#-------------------------------------------------------------------------------
# No options

#-------------------------------------------------------------------------------
# CMAKE VARIABLES
#-------------------------------------------------------------------------------
# No variable

#-------------------------------------------------------------------------------
# CMAKE CONFIGURATIONS
#-------------------------------------------------------------------------------
configure_file(include/packio/core/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/packio/core/version.h)

#-------------------------------------------------------------------------------
# Build module
#-------------------------------------------------------------------------------
add_library(packio_core INTERFACE)
target_include_directories(packio_core INTERFACE include/ ${CMAKE_CURRENT_BINARY_DIR}/generated/)
target_link_libraries(packio_core INTERFACE zlib)
add_library(packio ALIAS packio_core)