
project(testHttp)
set(CMAKE_BUILD_TYPE Release)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
link_libraries(http)
add_executable(httpserver test1.cc)

