#----------------------------------------------------------------------
# General settings
#
unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY)

if(ENABLE_THREAD_SANITIZER)
  add_compile_definitions(THREAD_SANITIZER)
endif()


#----------------------------------------------------------------------
# Public interface tests
#
set(XMSG_PUBLIC_TESTS
  context
  message
  topic
  util
)

foreach(name IN LISTS XMSG_PUBLIC_TESTS)
  add_executable(test_${name} ${name}_test.cpp)
  add_test(NAME test_${name} COMMAND test_${name} CONFIGURATIONS $<CONFIG> "")
  set_tests_properties(test_${name} PROPERTIES LABELS "unit;public")
  target_link_libraries(test_${name} PRIVATE xMsg::xmsg GTest::GMock)
endforeach()


#----------------------------------------------------------------------
# Internal implementation tests
#
set(XMSG_INTERNAL_TESTS
  connection_pool
  regdis
  zhelper
)

foreach(name IN LISTS XMSG_INTERNAL_TESTS)
  add_executable(test_${name} ${name}_test.cpp)
  add_test(NAME test_${name} COMMAND test_${name} CONFIGURATIONS $<CONFIG> "")
  set_tests_properties(test_${name} PROPERTIES LABELS "unit;private")
  target_include_directories(test_${name} PRIVATE "${PROJECT_SOURCE_DIR}/src/core")
  target_link_libraries(test_${name} PRIVATE xMsg::xmsg GTest::GMock)
endforeach()


#----------------------------------------------------------------------
# Slow integration tests
#
set(XMSG_INTEGRATION_TESTS
  subscription
)

foreach(name IN LISTS XMSG_INTEGRATION_TESTS)
  add_executable(test_${name} ${name}_test.cpp)
  add_test(NAME test_${name} COMMAND test_${name} CONFIGURATIONS Integration)
  set_tests_properties(test_${name} PROPERTIES LABELS "integration;slow" RUN_SERIAL TRUE)
  target_link_libraries(test_${name} PRIVATE xMsg::xmsg GTest::GMock)
endforeach()

add_custom_target(integration_test
  COMMAND ${CMAKE_CTEST_COMMAND} -C Integration)


#----------------------------------------------------------------------
# Standalone executable tests
#
add_executable(test_registration registrar_test.cpp)
target_link_libraries(test_registration PRIVATE xMsg::xmsg)
target_include_directories(test_registration PRIVATE "${PROJECT_SOURCE_DIR}/src/core")

add_executable(test_addr_resolution addr_resolution_test.cpp)
target_link_libraries(test_addr_resolution PRIVATE xMsg::xmsg)
