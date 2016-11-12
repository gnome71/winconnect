project(pluginManager)

# Do not automatically link Qt executables to qtmain target on Windows.
# Plugins use console mode only. Not need to link against qtmain.lib.
#cmake_policy(SET CMP0020 OLD)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(${PROJECT_NAME}_HEADERS
	plugininterface.h
	pluginmanager.h)

set(${PROJECT_NAME}_SOURCES
	pluginmanager.cpp)

add_library(${PROJECT_NAME} SHARED
	${${PROJECT_NAME}_SOURCES}
	${${PROJECT_NAME}_HEADERS})

generate_export_header(${PROJECT_NAME}
	EXPORT_FILE_NAME "${PROJECT_NAME}Export.h")

add_subdirectory(test)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
  COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "${${PROJECT_NAME}_BINARY_DIR}/${PROJECT_NAME}Export.h" "${CMAKE_BINARY_DIR}"
  COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "${${PROJECT_NAME}_BINARY_DIR}/${PROJECT_NAME}Export.h" "${CMAKE_BINARY_DIR}/${PROJECT_NAME}Export"
)

target_link_libraries(${PROJECT_NAME}
	Qt5::Core
)

