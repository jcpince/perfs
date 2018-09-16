cmake_minimum_required (VERSION 2.8.11)
project(libperfs "C")

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

add_library(perfs SHARED "")
add_library(perfs_static STATIC "")
set_target_properties(perfs_static PROPERTIES OUTPUT_NAME perfs)

target_sources(perfs
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/timestamper.c
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include/perfs/timestamper.h
)
target_sources(perfs_static
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/timestamper.c
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include/perfs/timestamper.h
)

add_executable(unit_test ${CMAKE_CURRENT_LIST_DIR}/test/test.c)
target_link_libraries(unit_test perfs)

add_executable(unit_test_static ${CMAKE_CURRENT_LIST_DIR}/test/test.c)
target_link_libraries(unit_test_static perfs_static)
