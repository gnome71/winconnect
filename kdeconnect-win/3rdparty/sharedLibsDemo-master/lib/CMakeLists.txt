#project(sharedLibs)

# create the shared library
add_library(shared SHARED shared.cpp)

target_include_directories(shared PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

# generate export header automatically
include (GenerateExportHeader)

GENERATE_EXPORT_HEADER(shared
             BASE_NAME shared
             EXPORT_MACRO_NAME SHARED_EXPORT
             EXPORT_FILE_NAME shared_EXPORTS.h
			 STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC
)
install(TARGETS shared
	RUNTIME DESTINATION ${CMAKE_BINARY_DIR}/bin
	LIBRARY DESTINATION ${CMAKE_BINARY_DIR}/lib
	ARCHIVE DESTINATION ${CMAKE_BINARY_DIR}/lib
)

