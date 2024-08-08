# set(SRC_LIST userservice.cc ../user.pb.cc)
set(SRC_LIST friendservice.cc ../friend.pb.cc)
add_executable(provider ${SRC_LIST})

# 链接我们需要的mprpc框架库（动态share库）和protobuf库给可执行文件provider
target_link_libraries(provider mprpc protobuf)