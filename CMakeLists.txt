cmake_minimum_required(VERSION 3.10)

project(bluecurve-gtk)

# we need to provide this option since GTK 2 is being removed from some distributions
option(COMPILE_GTK2_ENGINE "Compile and install the GTK 2 engine" ON)

find_package(PkgConfig REQUIRED)

# Set up GTK 3 (needed for installation directories)
pkg_check_modules(GTK3 gtk+-3.0)
if (NOT GTK3_FOUND)
  message(FATAL_ERROR "GTK 3 development libraries were not found.")
endif()
pkg_get_variable(GTK3_PREFIX gtk+-3.0 prefix)

# Set up GTK 2
if (COMPILE_GTK2_ENGINE)
  pkg_check_modules(GTK2 gtk+-2.0)
  if (NOT GTK2_FOUND)
    message(FATAL_ERROR "GTK 2 development libraries were not found.")
  endif()
  pkg_get_variable(GTK2_BINARY_VERSION gtk+-2.0 gtk_binary_version)
  pkg_get_variable(GTK2_LIBDIR gtk+-2.0 libdir)

  include_directories(${GTK2_INCLUDE_DIRS})
  add_compile_options(${GTK2_CFLAGS_OTHER})
  
  # set this to ignore all warnings, we have no time to fix the original GTK 2 code
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")

  # Compile GTK 2 engine
  add_library(bluecurve MODULE
    engine/bits.c
    engine/bluecurve_rc_style.c
    engine/bluecurve_style.c
    engine/bluecurve_theme_main.c
    engine/extract-alpha.c
  )
  target_link_libraries(bluecurve ${GTK2_LIBRARIES})

  # Install GTK 2 engine
  install(TARGETS bluecurve LIBRARY DESTINATION ${GTK2_LIBDIR}/gtk-2.0/${GTK2_BINARY_VERSION}/engines)
endif()

# Set up icon symlinks
set(BUILD_ICONS_DIR "${CMAKE_CURRENT_BINARY_DIR}/icon-set")
set(ICON_OUTPUT_FILE "${BUILD_ICONS_DIR}/.symlinks-generated")
file(GLOB_RECURSE ICONTHEME_FILES "${CMAKE_CURRENT_SOURCE_DIR}/icons/icon-xml/*.icontheme")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/icons/icon-set" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
add_custom_command(
  OUTPUT "${ICON_OUTPUT_FILE}"
  COMMAND python3 "${CMAKE_CURRENT_SOURCE_DIR}/icons/symlink.py" "${CMAKE_CURRENT_SOURCE_DIR}/icons/icon-xml" "${BUILD_ICONS_DIR}/Bluecurve"
  COMMAND gtk-update-icon-cache "${BUILD_ICONS_DIR}/Bluecurve"
  COMMAND ${CMAKE_COMMAND} -E touch "${ICON_OUTPUT_FILE}"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/icons/symlink.py" ${ICONTHEME_FILES}
)
add_custom_target(generate_icon_symlinks ALL DEPENDS "${ICON_OUTPUT_FILE}")

# Install icons
set(ICON_DIRS
  ${BUILD_ICONS_DIR}/Bluecurve
  ${BUILD_ICONS_DIR}/Bluecurve-inverse
  ${BUILD_ICONS_DIR}/LBluecurve
  ${BUILD_ICONS_DIR}/LBluecurve-inverse
)
foreach(ICON ${ICON_DIRS})
  install(DIRECTORY ${ICON} DESTINATION ${GTK3_PREFIX}/share/icons)
endforeach()

# Install GTK 2/3/4 theme files
set(THEME_DIRS
  themes/Bluecurve
  themes/Bluecurve-BerriesAndCream
  themes/Bluecurve-Classic-RH8
  themes/Bluecurve-Classic-RH9
  themes/Bluecurve-Gnome
  themes/Bluecurve-Grape
  themes/Bluecurve-Lime
  themes/Bluecurve-Slate
  themes/Bluecurve-Strawberry
  themes/Bluecurve-Tangerine
)
foreach(THEME ${THEME_DIRS})
  install(DIRECTORY ${THEME} DESTINATION ${GTK3_PREFIX}/share/themes)
endforeach()


