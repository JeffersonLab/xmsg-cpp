include_directories(${PROJECT_SOURCE_DIR}/src/core)
include_directories(${ZMQ_INCLUDE_DIRS})

add_executable(cx_proxy xmsg_proxy.cpp)
target_link_libraries(cx_proxy xmsg)

install(TARGETS cx_proxy DESTINATION ${CMAKE_INSTALL_BINDIR})
