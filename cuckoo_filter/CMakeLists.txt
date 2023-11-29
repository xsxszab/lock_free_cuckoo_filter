aux_source_directory(. LIB_SOURCES)

# build the cuckoo filter as a shared library
add_library(cuckoo_filter_shared ${LIB_SOURCES})
