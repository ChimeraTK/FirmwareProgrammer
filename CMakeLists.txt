cmake_minimum_required(VERSION 3.16)

project(ChimeraTK_FirmwareProgrammer)

set(${PROJECT_NAME}_MAJOR_VERSION 04)
set(${PROJECT_NAME}_MINOR_VERSION 09)
set(${PROJECT_NAME}_PATCH_VERSION 00)

## Set additional version-specific variables
include(${CMAKE_SOURCE_DIR}/cmake/set_version_numbers.cmake)
## Load custom modules, i.e. FindDOOCS.cmake
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

INCLUDE(${CMAKE_SOURCE_DIR}/cmake/set_default_build_to_release.cmake)
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/set_default_flags.cmake)

include(cmake/add_dependency.cmake)
add_dependency(ChimeraTK-DeviceAccess 03.08 REQUIRED)
add_dependency(Boost 1.46 program_options REQUIRED)

include(cmake/enable_code_style_check.cmake)

#Add Curses dependency
set(CURSES_USE_NCURSES TRUE)
FIND_PACKAGE(Curses)
include_directories(SYSTEM ${CURSES_INCLUDE_DIR})


INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
AUX_SOURCE_DIRECTORY(${CMAKE_SOURCE_DIR}/src app_sources)

#put the version number into the version file and add the include directory in the build tree, where we put it
configure_file(cmake/version.h.in
  "${PROJECT_BINARY_DIR}/include/version.h" @ONLY)
include_directories(${PROJECT_BINARY_DIR}/include)

set(EXEC_NAME mtca4u_fw_programmer)
add_executable(${EXEC_NAME} ${app_sources} )
set_target_properties(${EXEC_NAME} PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS}")
set_target_properties(${EXEC_NAME} PROPERTIES LINK_FLAGS "${ChimeraTK-DeviceAccess_LINK_FLAGS} ${Boost_LINK_FLAGS} ${CURSES_LINK_FLAGS}")
target_link_libraries(${EXEC_NAME} ${ChimeraTK-DeviceAccess_LIBRARIES} ${Boost_LIBRARIES} ${CURSES_LIBRARIES})

#change the install prefix to the source directory in case the user has not specified a destination
#i. e. CMAKE_INSTALL_PREFIX is not set manually
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install CACHE PATH "Install directory
                 prefix" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

#Install the library and the executables
install(TARGETS ${EXEC_NAME}
  RUNTIME DESTINATION bin
  )

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  if(NOT EXISTS "${DOXROOT}")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doc")
  endif()

  add_custom_target(doc ALL
    ${DOXYGEN_EXECUTABLE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
  # do not install the developer information
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/ DESTINATION share/doc/${PROJECT_NAME})
else(DOXYGEN_FOUND)
  message("Doxygen not found, documentation will not be build.")
endif(DOXYGEN_FOUND)
