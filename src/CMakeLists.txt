# Define source files.
set(CONFETTI_SOURCES
    "list.c"
    "linked_list.c"
)

# Define header files.
set(CONFETTI_HEADERS
    "include/list.h" 
    "include/linked_list.h"
)

# include required packages.
include(GNUInstallDirs)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)

# Define confetti with it's sources and headers.
add_library(confetti ${CONFETTI_SOURCES} ${CONFETTI_HEADERS})

# Sets confetti properties.
set_target_properties(confetti 
    PROPERTIES
    CMAKE_C_STANDARD 99
    CMAKE_C_STANDARD_REQUIRED ON
    CMAKE_C_EXTENSIONS OFF
)

# Set include directories for confetti
target_include_directories(confetti 
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
)

# Create confetti_export.h
generate_export_header(confetti)

# Set install paths for confetti
install(
    TARGETS confetti
    EXPORT "confettiTargets"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# Install confetti header files and confetti_export.h to CMAKE_PREFIX_PATH/include
install(
    FILES 
        ${CONFETTI_HEADERS} 
        "${CMAKE_CURRENT_BINARY_DIR}/confetti_export.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT Development
)

# Create version file. 
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/confetti/confettiConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}  
    COMPATIBILITY SameMajorVersion
)

# Export confettiTargets for other projects to easily find and link against confetti::confetti.
export(
    EXPORT "confettiTargets"
    NAMESPACE confetti::
    FILE "${CMAKE_CURRENT_BINARY_DIR}/confetti/confettiTargets.cmake"
)

# Define where config files should be installed to
set(ConfettiConfigPath ${CMAKE_INSTALL_LIBDIR}/cmake/confetti)

# Create a package config file for confetti from a template file.
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/confettiConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/confettiConfig.cmake"
  INSTALL_DESTINATION ${ConfettiConfigPath}
)

# Install target files
install(
    EXPORT "confettiTargets" 
    FILE "confettiTargets.cmake"
    NAMESPACE confetti::
    DESTINATION ${ConfettiConfigPath}
)

# Install config files
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/confettiConfig.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/confetti/confettiConfigVersion.cmake"  
    DESTINATION ${ConfettiConfigPath} 
    COMPONENT Development
)

# Install the copying.lesser license file
install(
    FILES ../COPYING.LESSER 
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    COMPONENT Development
)
