set(SRC_LIST callfriendservice.cc ../friend.pb.cc)

add_executable(consumer ${SRC_LIST})
target_link_libraries(consumer myrpc protobuf)