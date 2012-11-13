# generated by Buildyard, do not edit.

include(System)
set(FIND_PACKAGES_FOUND ${SYSTEM} ${FIND_PACKAGES_FOUND_EXTRA})

find_package(OFED )
if(OFED_FOUND)
  set(OFED_name OFED)
elseif(OFED_FOUND)
  set(OFED_name OFED)
endif()
if(OFED_name)
  list(APPEND FIND_PACKAGES_FOUND COLLAGE_USE_OFED)
  link_directories(${${OFED_name}_LIBRARY_DIRS})
  include_directories(${${OFED_name}_INCLUDE_DIRS})
endif()

find_package(UDT )
if(UDT_FOUND)
  set(UDT_name UDT)
elseif(UDT_FOUND)
  set(UDT_name UDT)
endif()
if(UDT_name)
  list(APPEND FIND_PACKAGES_FOUND COLLAGE_USE_UDT)
  link_directories(${${UDT_name}_LIBRARY_DIRS})
  include_directories(${${UDT_name}_INCLUDE_DIRS})
endif()

find_package(Boost 1.41.0 REQUIRED system regex date_time serialization)
if(Boost_FOUND)
  set(Boost_name Boost)
elseif(BOOST_FOUND)
  set(Boost_name BOOST)
endif()
if(Boost_name)
  list(APPEND FIND_PACKAGES_FOUND COLLAGE_USE_BOOST)
  link_directories(${${Boost_name}_LIBRARY_DIRS})
  include_directories(SYSTEM ${${Boost_name}_INCLUDE_DIRS})
endif()

find_package(Lunchbox 1.5.0 REQUIRED)
if(Lunchbox_FOUND)
  set(Lunchbox_name Lunchbox)
elseif(LUNCHBOX_FOUND)
  set(Lunchbox_name LUNCHBOX)
endif()
if(Lunchbox_name)
  list(APPEND FIND_PACKAGES_FOUND COLLAGE_USE_LUNCHBOX)
  link_directories(${${Lunchbox_name}_LIBRARY_DIRS})
  include_directories(${${Lunchbox_name}_INCLUDE_DIRS})
endif()


# Write defines.h and options.cmake
if(NOT FIND_PACKAGES_INCLUDE)
  set(FIND_PACKAGES_INCLUDE
    "${CMAKE_BINARY_DIR}/include/${CMAKE_PROJECT_NAME}/defines${SYSTEM}.h")
endif()
if(NOT OPTIONS_CMAKE)
  set(OPTIONS_CMAKE ${CMAKE_BINARY_DIR}/options.cmake)
endif()
set(DEFINES_FILE ${FIND_PACKAGES_INCLUDE})
set(DEFINES_FILE_IN ${DEFINES_FILE}.in)
file(WRITE ${DEFINES_FILE_IN}
  "// generated by Buildyard, do not edit.\n\n"
  "#ifndef ${CMAKE_PROJECT_NAME}_DEFINES_${SYSTEM}_H\n"
  "#define ${CMAKE_PROJECT_NAME}_DEFINES_${SYSTEM}_H\n\n")
file(WRITE ${OPTIONS_CMAKE} "# Optional modules enabled during build\n")
foreach(DEF ${FIND_PACKAGES_FOUND})
  add_definitions(-D${DEF})
  file(APPEND ${DEFINES_FILE_IN}
  "#ifndef ${DEF}\n"
  "#  define ${DEF}\n"
  "#endif\n")
if(NOT DEF STREQUAL SYSTEM)
  file(APPEND ${OPTIONS_CMAKE} "set(${DEF} ON)\n")
endif()
endforeach()
file(APPEND ${DEFINES_FILE_IN}
  "\n#endif\n")

include(UpdateFile)
update_file(${DEFINES_FILE_IN} ${DEFINES_FILE})
