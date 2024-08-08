# 设置cmake的最低要求版本
cmake_minimum_required(VERSION 3.0)
# 设置项目的名称
project(mprpc)

# 设置项目可执行文件的输出路径(使用框架生成的是可执行文件)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 设置项目库文件输出的路径(我们本身是开发一个rpc框架，最后生成的应该都是lib库文件)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置项目编译头文件的搜索路径
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)
# 设置项目库文件搜索路径
link_directories(${PROJECT_SOURCE_DIR}/lib)

# 添加子目录
# src包含了mprpc框架的所有相关代码
add_subdirectory(src)
# example包含的是mprpc框架使用的示例代码
add_subdirectory(example)