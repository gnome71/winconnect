project(pingPlugin)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

add_definitions(-DQT_PLUGIN)

set(pingPlugin_HDRS pinglugin.h)
set(pingPlugin_SRCS pingplugin.cpp)
set(pingPlugin_MISC	pingPlugin.json)

add_library(${PROJECT_NAME} SHARED
  pingplugin.h
  pingplugin.cpp
  pingPlugin.json
)

generate_export_header(${PROJECT_NAME}
	 EXPORT_FILE_NAME "${PROJECT_NAME}Export.h")

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
  COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "${${PROJECT_NAME}_BINARY_DIR}/${PROJECT_NAME}Export.h" "${CMAKE_BINARY_DIR}"
  COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "${${PROJECT_NAME}_BINARY_DIR}/${PROJECT_NAME}Export.h" "${CMAKE_BINARY_DIR}/${PROJECT_NAME}Export"
)

target_link_libraries(${PROJECT_NAME}
	#${QtCore_location}
	Qt5::Core
	coreLib
	pluginManager
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Plugins")
source_group("Source Files" FILES pingplugin.h ${PROJECT_NAME}Export.h pingplugin.cpp)
source_group("Misc Files" FILES pingPlugin.json)
