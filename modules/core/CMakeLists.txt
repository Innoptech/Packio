message(STATUS "Adding serialize::core module")

#-------------------------------------------------------------------------------
# Ensure requirements
#-------------------------------------------------------------------------------
# No requirements

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
configure_file(include/serialpack/core/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/serialpack/core/version.h)

#-------------------------------------------------------------------------------
# Build module
#-------------------------------------------------------------------------------
add_library(serialize_core INTERFACE)
target_include_directories(serialize_core INTERFACE include/ ${CMAKE_CURRENT_BINARY_DIR}/generated/)
add_library(serialize::core ALIAS serialize_core)