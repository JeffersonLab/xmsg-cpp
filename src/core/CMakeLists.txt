include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(XMSG_HEADERS
  xmsg_constants.h
  xmsg_util.h
)

set(XMSG_FILES
  xmsg_util.cpp
)

add_library(xmsg SHARED ${XMSG_FILES})
target_link_libraries(xmsg xmsgproto)

install(FILES ${XMSG_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS xmsg DESTINATION ${CMAKE_INSTALL_LIBDIR})
