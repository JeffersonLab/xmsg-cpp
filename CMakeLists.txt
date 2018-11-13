cmake_minimum_required(VERSION 3.5.1)
project(xMsg LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    Debug Release MinSizeRel RelWithDebInfo)
endif()

include(GNUInstallDirs)


find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wno-missing-braces)

if(APPLE)
  set(CMAKE_MACOSX_RPATH ON)
  set(CMAKE_SKIP_BUILD_RPATH FALSE)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir)
  if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
  endif()
endif()


list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

set(XMSG_ZEROMQ_MIN_VERSION 4.0)
set(XMSG_PROTOBUF_MIN_VERSION 2.5)

find_package(ZeroMQ ${XMSG_ZEROMQ_MIN_VERSION} REQUIRED)
find_package(Protobuf ${XMSG_PROTOBUF_MIN_VERSION} REQUIRED)

include(EnsureProtobufTarget)

string(CONCAT PKGCONF_REQ_PUB
  "libzmq >= ${XMSG_ZEROMQ_MIN_VERSION}, "
  "protobuf >= ${XMSG_PROTOBUF_MIN_VERSION}")


option(XMSG_BUILD_TESTS "Build tests" ON)

add_subdirectory(src/core)
add_subdirectory(src/proto)
add_subdirectory(src/sys)
add_subdirectory(doc)

if(XMSG_BUILD_TESTS)
  enable_testing()
  include(cmake/External/GTest.cmake)
  add_subdirectory(test/core)
  add_subdirectory(test/perf)
endif()


# package configuration
set(CMAKE_INSTALL_CONFIGDIR "${CMAKE_INSTALL_LIBDIR}/cmake/xmsg")

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/xmsg-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/xmsg-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_CONFIGDIR}
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/xmsg-config.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/FindZeroMQ.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/EnsureProtobufTarget.cmake
  DESTINATION ${CMAKE_INSTALL_CONFIGDIR}
  COMPONENT Devel)


# export targets
export(EXPORT xMsgTargets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/xmsg-targets.cmake
  NAMESPACE xMsg::
)

install(EXPORT xMsgTargets
  FILE xmsg-targets.cmake
  NAMESPACE xMsg::
  DESTINATION ${CMAKE_INSTALL_CONFIGDIR}
  COMPONENT Devel)


# pkg-config
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/xmsg.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/xmsg.pc
  @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/xmsg.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
  COMPONENT Devel)


# uninstall target
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
  @ONLY)

add_custom_target(uninstall
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
