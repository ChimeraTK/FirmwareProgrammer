cmake_minimum_required(VERSION 2.8.7)

project(ChimeraTK_FirmwareProgrammer)

set(${PROJECT_NAME}_MAJOR_VERSION 04)
set(${PROJECT_NAME}_MINOR_VERSION 03)
set(${PROJECT_NAME}_PATCH_VERSION 00)

## Set additional version-specific variables
include(${CMAKE_SOURCE_DIR}/cmake/set_version_numbers.cmake)
## Load custom modules, i.e. FindDOOCS.cmake
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "None")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
    "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." 
    FORCE)
endif(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "None")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -Wextra -Wshadow -pedantic -Wuninitialized -Weffc++")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 --coverage")

include(cmake/add_dependency.cmake)
add_dependency(ChimeraTK-DeviceAccess 1.00 REQUIRED)
add_dependency(Boost 1.46 program_options REQUIRED)

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

