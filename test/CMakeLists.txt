file(GLOB TEST_SOURCES "${PROJECT_SOURCE_DIR}/test/*.cpp")

message(STATUS "configure cmake in directory ./test")

find_package(OpenSSL REQUIRED)

foreach(SOURCE ${TEST_SOURCES})
    get_filename_component( SOURCE_FILENAME ${SOURCE} NAME)
    string(REPLACE ".cpp" "" OUTPUT_NAME  ${SOURCE_FILENAME})
    add_executable(${OUTPUT_NAME} ${SOURCE})
    target_link_libraries(${OUTPUT_NAME} cuckoo_filter_shared OpenSSL::SSL)

endforeach(SOURCE ${TEST_SOURCES})
