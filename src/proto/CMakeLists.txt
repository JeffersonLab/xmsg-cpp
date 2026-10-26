# Force C++17 for everything in this directory
add_compile_options(-std=c++17)

set(PROTO_FILES
  data.proto
  meta.proto
  registration.proto
)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_library(xmsgproto_obj OBJECT ${PROTO_SRCS})

target_compile_options(xmsgproto_obj PRIVATE -Wno-extra)

if(BUILD_SHARED_LIBS)
  set_target_properties(xmsgproto_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

target_include_directories(xmsgproto_obj SYSTEM PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(xmsgproto INTERFACE)
target_link_libraries(xmsgproto INTERFACE protobuf::libprotobuf)
target_include_directories(xmsgproto SYSTEM INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

install(FILES ${PROTO_HDRS}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xmsg/proto
  COMPONENT xMsg_Devel
)
install(TARGETS xmsgproto EXPORT xMsgTargets)

