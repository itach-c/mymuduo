
project(testTcp)

#set(CMAKE_BUILD_TYPE Release)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
link_libraries(tcp)
add_executable(tcpserver server.cc)

