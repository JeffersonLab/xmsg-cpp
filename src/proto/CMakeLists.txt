set(PROTO_FILES
  data.proto
  meta.proto
  registration.proto
)

PROTOBUF_GENERATE_CPP(PROTO_SOURCES PROTO_HEADERS ${PROTO_FILES})

add_library(xmsgproto OBJECT ${PROTO_SOURCES})
target_compile_options(xmsgproto PRIVATE -Wno-extra)
set_target_properties(xmsgproto PROPERTIES
  CXX_CLANG_TIDY ""
  POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})
target_include_directories(xmsgproto SYSTEM PUBLIC
  $<TARGET_PROPERTY:protobuf::libprotobuf,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(xmsgproto_headers INTERFACE)
target_link_libraries(xmsgproto_headers INTERFACE protobuf::libprotobuf)
target_include_directories(xmsgproto_headers SYSTEM INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

install(FILES ${PROTO_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xmsg/proto)
install(TARGETS xmsgproto_headers EXPORT xMsgTargets)
