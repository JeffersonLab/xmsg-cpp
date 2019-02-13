set(PROTO_FILES
  data.proto
  meta.proto
  registration.proto
)

protobuf_generate_cpp(PROTO_SOURCES PROTO_HEADERS ${PROTO_FILES})

add_library(xmsgproto_obj OBJECT ${PROTO_SOURCES})
target_compile_options(xmsgproto_obj PRIVATE -Wno-extra)
set_target_properties(xmsgproto_obj PROPERTIES CXX_CLANG_TIDY "")
if(BUILD_SHARED_LIBS)
  set_target_properties(xmsgproto_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
target_include_directories(xmsgproto_obj SYSTEM PUBLIC
  $<TARGET_PROPERTY:xmsgproto,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(xmsgproto INTERFACE)
target_link_libraries(xmsgproto INTERFACE protobuf::libprotobuf)
target_include_directories(xmsgproto SYSTEM INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

install(FILES ${PROTO_HEADERS}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xmsg/proto
  COMPONENT xMsg_Devel
)
install(TARGETS xmsgproto EXPORT xMsgTargets)
