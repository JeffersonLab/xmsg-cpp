include_directories(${GMOCK_INCLUDE_DIRS})

add_executable(test_topic xmsg_topic_test.cpp)
add_dependencies(test_topic googlemock)
add_test(test_topic test_topic)
target_link_libraries(test_topic xmsg ${GMOCK_LIBRARIES})

add_executable(test_util xmsg_util_test.cpp)
add_dependencies(test_util googlemock)
add_test(test_util test_util)
target_link_libraries(test_util xmsg ${GMOCK_LIBRARIES})

add_executable(test_subscription test_subscription.cpp)
add_dependencies(test_subscription googlemock)
target_link_libraries(test_subscription xmsg ${GMOCK_LIBRARIES})

add_executable(test_regdis xmsg_regdis_test.cpp)
add_dependencies(test_regdis googlemock)
add_test(test_regdis test_regdis)
target_link_libraries(test_regdis xmsg ${GMOCK_LIBRARIES})

add_executable(test_registration test_registration.cpp)
add_dependencies(test_registration googlemock)
target_link_libraries(test_registration xmsg ${GMOCK_LIBRARIES})
