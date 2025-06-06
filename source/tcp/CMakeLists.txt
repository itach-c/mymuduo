project(tcp)

set(LIBRARY_OUTPUT_PATH ${LIBDIR})

aux_source_directory(${PROJECT_SOURCE_DIR} SRC)

include_directories(${PROJECT_SOURCE_DIR}/../comm/)

add_library(tcp SHARED ${SRC})

# 链接tcmalloc库
#target_link_libraries(tcp tcmalloc)