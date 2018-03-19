cmake_minimum_required (VERSION 3.1)
project (Basics)

# Required to setup glib-2.0
find_package(PkgConfig REQUIRED)
# INCLUDE(FindPkgConfig)

# Basics project version number
set (Basics_VERSION_MAJOR 0)
set (Basics_VERSION_MINOR 1)
set (Basics_VERSION_SUB 0)

configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )

pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0 gio-unix-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
link_directories(${GLIB2_LIBRARY_DIRS})

include_directories("${PROJECT_BINARY_DIR}")

find_program(GDBUSCODEGEN NAMES gdbus-codegen)
if (NOT GDBUSCODEGEN)
  message(SEND_ERROR "Couldn't find gdbus-codegen")
endif (NOT GDBUSCODEGEN)
add_custom_command(OUTPUT basics_gen.c basics_gen.h
  COMMAND ${GDBUSCODEGEN} --generate-c-code=basics_gen ${PROJECT_SOURCE_DIR}/basics.gdbus.xml
  DEPENDS basics.gdbus.xml
  )

add_executable(basics_client basics_client.c basics_gen.c)
target_link_libraries(basics_client ${GLIB2_LIBRARIES})

add_executable(basics_server basics_server.c basics_gen.c)
target_link_libraries(basics_server ${GLIB2_LIBRARIES})
