include_directories(${ZMQ_INCLUDE_DIRS})

set(XMSG_HEADERS
  xmsg_address.h
  xmsg_constants.h
  xmsg_connection.h
  xmsg_connection_pool.h
  xmsg_connection_setup.h
  xmsg_message.h
  xmsg_topic.h
  xmsg_util.h
)

set(XMSG_FILES
  xmsg_address.cpp
  xmsg_connection.cpp
  xmsg_connection_pool.cpp
  xmsg_connection_setup.cpp
  xmsg_regdis.cpp
  xmsg_topic.cpp
  xmsg_util.cpp
)

add_library(xmsg SHARED ${XMSG_FILES} $<TARGET_OBJECTS:xmsgproto>)
target_link_libraries(xmsg xmsgproto_headers ${ZMQ_LIBRARIES})
target_include_directories(xmsg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(FILES ${XMSG_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS xmsg DESTINATION ${CMAKE_INSTALL_LIBDIR})
