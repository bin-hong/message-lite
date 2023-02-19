
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# my compile env has not gtest,now commet it
#set(Gtest_message_event_handle_SRCS ./Gtest_message_event_handle.cpp)
#add_executable(Gtest_message_event_handle ${Gtest_message_event_handle_SRCS})
#target_link_libraries(Gtest_message_event_handle )

set(test_users_SRC 
    ${CMAKE_SOURCE_DIR}/test/test_users.c 
    ${CMAKE_SOURCE_DIR}/src/message_users.c
    ${CMAKE_SOURCE_DIR}/src/message_log.c
    ${CMAKE_SOURCE_DIR}/src/message_msgs.c
    ${CMAKE_SOURCE_DIR}/src/message_event_handle.c
    )

add_executable(test_users ${test_users_SRC})
target_link_libraries(test_users)

set(test_msgs_SRC 
    ${CMAKE_SOURCE_DIR}/test/test_msgs.c
    ${CMAKE_SOURCE_DIR}/src/message_users.c
    ${CMAKE_SOURCE_DIR}/src/message_log.c
    ${CMAKE_SOURCE_DIR}/src/message_msgs.c
    ${CMAKE_SOURCE_DIR}/src/message_event_handle.c
    )
add_executable(test_msgs ${test_msgs_SRC})
target_link_libraries(test_msgs)
