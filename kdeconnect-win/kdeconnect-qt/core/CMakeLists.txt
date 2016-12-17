project(kdeconnectcore)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Qca_INCLUDES}
)

add_subdirectory(backends/lan)
#add_subdirectory(backends/loopback)

set(core_SRCS
	${backends_SRCS}

	core/backends/linkprovider.cpp
	core/backends/devicelink.cpp
	core/backends/pairinghandler.cpp

	#core/kclogger.cpp
	core/kdeconnectconfig.cpp
	core/kdeconnectplugin.cpp
	core/kdeconnectpluginconfig.cpp
	core/deviceidhelper.cpp
	core/networkpackage.cpp
	core/daemon.cpp
	core/device.cpp

	PARENT_SCOPE
	)

set(core_HDRS
	${backends_HDRS}

	#core/kclogger.h
	core/kdeconnectconfig.h
	core/kdeconnectplugin.h
	core/kdeconnectpluginconfig.h
	core/deviceidhelper.h
	core/networkpackage.h
	core/daemon.h
	core/device.h

	PARENT_SCOPE
)

# Start DLL settings
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(coredll_SRCS
	kclogger.cpp
)
set(coredll_HDRS
	kclogger.h
)

add_library(kcLogger SHARED
	${coredll_SRCS}
	${coredll_HDRS}
)

target_include_directories(kcLogger PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}	${CMAKE_CURRENT_SOURCE_DIR}
)
generate_export_header(kcLogger EXPORT_FILE_NAME "kcLoggerExport.h")

add_custom_command(TARGET kcLogger PRE_BUILD
  COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different 
	"${CMAKE_CURRENT_BINARY_DIR}/kcLoggerExport.h" 
	"${CMAKE_BINARY_DIR}"
  COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different 
	"${CMAKE_CURRENT_BINARY_DIR}/kcLoggerExport.h" 
	"${CMAKE_BINARY_DIR}/kcLoggerExport"
)

target_link_libraries(kcLogger
	Qt5::Core
)
set_property(TARGET kcLogger PROPERTY FOLDER "Libraries")
