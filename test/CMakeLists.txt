file(GLOB TEST_SOURCES "${PROJECT_SOURCE_DIR}/test/*.cpp")
find_library(filter_shard_lib libcuckoo_filter_shared.so ${CMAKE_BINARY_DIR}/lib/)

message(STATUS "configure cmake in directory ./test")

foreach(SOURCE ${TEST_SOURCES})
    get_filename_component( SOURCE_FILENAME ${SOURCE} NAME)
    string(REPLACE ".cpp" "" OUTPUT_NAME  ${SOURCE_FILENAME})
    add_executable(${OUTPUT_NAME} EXCLUDE_FROM_ALL ${SOURCE})
    target_link_libraries(${OUTPUT_NAME} ${filter_shard_lib})

endforeach(SOURCE ${TEST_SOURCES})
