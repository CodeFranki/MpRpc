# 将本目录下的所有cc文件存在变量中
aux_source_directory(. SRC_LIST)

# 生成share动态库
add_library(mprpc ${SRC_LIST})
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt) # zookeeper_st表示单线程版本，mt表示多线程版本