set(XMSG_FILES
  xmsg.cpp
  address.cpp
  context.cpp
  connection_driver.cpp
  connection_pool.cpp
  connection_setup.cpp
  proxy.cpp
  registration_driver.cpp
  topic.cpp
  subscription.cpp
  util.cpp
  zhelper.cpp
)

add_library(xmsg ${XMSG_FILES} $<TARGET_OBJECTS:xmsgproto_obj>)

target_link_libraries(xmsg xmsgproto ZeroMQ::libzmq)
target_include_directories(xmsg PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_include_directories(xmsg SYSTEM PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/third_party>)

if(ENABLE_THREAD_SANITIZER)
  target_compile_options(xmsg PUBLIC ${THREAD_SANITIZER})
  target_link_libraries(xmsg ${THREAD_SANITIZER})
endif()

add_library(xMsg::xmsg ALIAS xmsg)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/xmsg/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xmsg
  COMPONENT xMsg_Devel
)
install(TARGETS xmsg EXPORT xMsgTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT xMsg_Runtime
)
