project(batteryPlugin)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

add_definitions(-DQT_PLUGIN)

add_library(${PROJECT_NAME} SHARED
  batteryplugin.h
  batteryplugin.cpp
  batteryPlugin.json
)

generate_export_header(${PROJECT_NAME}
	 EXPORT_FILE_NAME "${PROJECT_NAME}Export.h")

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
  COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "${${PROJECT_NAME}_BINARY_DIR}/${PROJECT_NAME}Export.h" "${CMAKE_BINARY_DIR}"
  COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "${${PROJECT_NAME}_BINARY_DIR}/${PROJECT_NAME}Export.h" "${CMAKE_BINARY_DIR}/${PROJECT_NAME}Export"
)

target_link_libraries(${PROJECT_NAME}
	Qt5::Core
	coreLib
	pluginManager
)

set(MISC
	batteryPlugin.json
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Plugins")
source_group("Source Files" FILES batteryplugin.h ${PROJECT_NAME}Export.h batteryplugin.cpp)
source_group("Misc Files" FILES ${MISC})
