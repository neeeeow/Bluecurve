# Minimum CMake version
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(Bluecurve-GTK2-engine C)

# Enable the creation of a shared library (the .so file)
set(BUILD_SHARED_LIBS ON)

# Find GTK 2
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK2 gtk+-2.0)

# Custom error message if GTK 2 is not found
if (NOT GTK2_FOUND)
    message(FATAL_ERROR "GTK 2 development libraries were not found. Please install GTK 2 development packages.")
endif()

# Include directories for GTK
include_directories(${GTK2_INCLUDE_DIRS})

# Compiler flags for GTK
add_compile_options(${GTK2_CFLAGS_OTHER})

# Source files for the engine
set(SOURCE_FILES
    bits.c
    bluecurve_rc_style.c
    bluecurve_style.c
    bluecurve_theme_main.c
    extract-alpha.c
)

# Create the shared library (.so)
add_library(bluecurve SHARED ${SOURCE_FILES})

# Link the GTK libraries
target_link_libraries(bluecurve ${GTK2_LIBRARIES})

# Install the library to the specified location
install(TARGETS bluecurve LIBRARY DESTINATION ~/.gtk-2.0/engines)

