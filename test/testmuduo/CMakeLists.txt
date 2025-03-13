
project(testmuduo)
set(CMAKE_BUILD_TYPE Release)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)

link_directories(./lib)
link_libraries(muduo_net muduo_base muduo_http)
add_executable(server test.cc)




