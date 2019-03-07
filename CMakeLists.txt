cmake_minimum_required(VERSION 3.5.1)
project(xMsg LANGUAGES CXX)

set(MASTER_PROJECT OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(MASTER_PROJECT ON)
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    Debug Release MinSizeRel RelWithDebInfo)
endif()

if(NOT ${CMAKE_VERSION} VERSION_LESS 3.11 OR FETCHCONTENT_SOURCE_DIR_XMSG)
  option(XMSG_BUILD_TESTS "Build tests" ${MASTER_PROJECT})
elseif(NOT XMSG_TESTS_DISABLED OR XMSG_BUILD_TESTS)
  message(STATUS "Tests are disabled (CMake >= 3.11 required)")
  set(XMSG_TESTS_DISABLED ON CACHE INTERNAL "" FORCE)
  set(XMSG_BUILD_TESTS OFF)
endif()

option(XMSG_BUILD_DOCS "Build docs" ${MASTER_PROJECT})

if(NOT DEFINED BUILD_SHARED_LIBS)
  set(BUILD_SHARED_LIBS ON)
endif()

include(GNUInstallDirs)

if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
endif()
if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
endif()
if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
endif()


if(MASTER_PROJECT)
  find_program(CCACHE_PROGRAM ccache)
  if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" CACHE PATH "")
  endif()

  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

  option(ENABLE_THREAD_SANITIZER "Enable thread-sanitizer support" OFF)

  if(ENABLE_THREAD_SANITIZER)
    set(THREAD_SANITIZER -fsanitize=thread -fno-omit-frame-pointer)
  endif()
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wno-missing-braces)

if(APPLE)
  set(CMAKE_MACOSX_RPATH ON)
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  file(RELATIVE_PATH relDir
    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}
    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}
  )
  set(CMAKE_INSTALL_RPATH @loader_path @loader_path/${relDir})
endif()


list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

if(NOT ${CMAKE_VERSION} VERSION_LESS 3.11)
  set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/ThirdParty" CACHE PATH
    "Directory under which to collect all populated content")
  include(FetchContent)
endif()

set(XMSG_ZEROMQ_MIN_VERSION 4.0)
set(XMSG_PROTOBUF_MIN_VERSION 2.5)

find_package(ZeroMQ ${XMSG_ZEROMQ_MIN_VERSION} REQUIRED)
find_package(Protobuf ${XMSG_PROTOBUF_MIN_VERSION} REQUIRED)

include(EnsureProtobufTarget)

string(CONCAT PKGCONF_REQ_PUB
  "libzmq >= ${XMSG_ZEROMQ_MIN_VERSION}, "
  "protobuf >= ${XMSG_PROTOBUF_MIN_VERSION}")


add_subdirectory(src/core)
add_subdirectory(src/proto)
add_subdirectory(src/sys)

if(XMSG_BUILD_DOCS)
  add_subdirectory(doc)
endif()

if(XMSG_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test/third_party)
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
  COMPONENT xMsg_Devel)


# export targets
export(EXPORT xMsgTargets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/xmsg-targets.cmake
  NAMESPACE xMsg::
)

install(EXPORT xMsgTargets
  FILE xmsg-targets.cmake
  NAMESPACE xMsg::
  DESTINATION ${CMAKE_INSTALL_CONFIGDIR}
  COMPONENT xMsg_Devel)


# pkg-config
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/xmsg.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/xmsg.pc
  @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/xmsg.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
  COMPONENT xMsg_Devel)


# uninstall target
if(MASTER_PROJECT)
  configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
