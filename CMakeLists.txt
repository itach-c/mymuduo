cmake_minimum_required(VERSION 3.22)

project(mymuduo)
add_definitions(-DLOG_LEVEL=0)

# 修正拼写错误，添加调试信息
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_STANDARD 17)

set(LIBDIR ${PROJECT_SOURCE_DIR}/lib)
add_subdirectory(./source/http)
add_subdirectory(./source/tcp)

# include 
include_directories(./source/http)
include_directories(./source/tcp)
include_directories(./source/commn)  # 注意：这里可能也有拼写错误，应该是common?

# 测试路径
add_subdirectory(./test/testHTTP)
add_subdirectory(./test/testmuduo)
add_subdirectory(./test/testTcp)