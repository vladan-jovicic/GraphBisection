cmake_minimum_required(VERSION 3.5)
project(GraphBisection)

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})


set(CMAKE_C_COMPILE_FLAGS ${CMAKE_C_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
set(CMAKE_C_LINK_FLAGS ${CMAKE_C_LINK_FLAGS} ${MPI_LINK_FLAGS})

set(SOURCE_FILES
    common/init.h common/init.c common/cut_funcs.h main.c common/read.h common/read.c common/cut_funcs.c sequential/sequential_gblp.h all-to-all/all2all_algo.h peer-to-peer/peer2peer_algo.h sequential/sequential_algo.c common/graph_funcs.h common/graph_funcs.c all-to-all/all2all_algo.c peer-to-peer/peer2peer_algo.c)

add_executable(GraphBisection ${SOURCE_FILES})
target_link_libraries(GraphBisection ${MPI_C_LIBRARIES})
target_link_libraries(GraphBisection m)





