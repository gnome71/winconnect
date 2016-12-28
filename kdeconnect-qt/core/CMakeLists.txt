project(kdeconnectcore)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Qca_INCLUDES}
)

add_subdirectory(backends/lan)
add_subdirectory(backends/loopback)

# Start DLL settings
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Set lib sources
set(coredll_SRCS
	daemon.cpp
	device.cpp
	kclogger.cpp
	kdeconnectconfig.cpp
	kdeconnectpluginconfig.cpp
	kdeconnectplugin.cpp
	networkpackage.cpp)

set(coredll_be_SRCS
	backends/devicelink.cpp
	backends/linkprovider.cpp
	backends/pairinghandler.cpp)

set(coredll_be_lan_SRCS
	${backends_SRCS})

set(coredll_be_loop_SRCS
	${backends_loop_SRCS})

# Set lib headers
set(coredll_HDRS
	daemon.h
	device.h
	kclogger.h
	kdeconnectconfig.h
	kdeconnectpluginconfig.h
	kdeconnectplugin.h
	networkpackage.h)

set(coredll_be_HDRS
	backends/devicelink.h
	backends/linkprovider.h
	backends/pairinghandler.h)

set(coredll_be_lan_HDRS
	${backends_HDRS})

set(coredll_be_loop_HDRS
	${backends_loop_HDRS})

# Build Lib
add_library(coreLib SHARED
	${coredll_SRCS}
	${coredll_HDRS}
	${coredll_be_SRCS}
	${coredll_be_HDRS}
	${coredll_be_lan_SRCS}
	${coredll_be_lan_HDRS}
	${coredll_be_loop_SRCS}
	${coredll_be_loop_HDRS}
)

target_include_directories(coreLib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}	${CMAKE_CURRENT_SOURCE_DIR}
)
generate_export_header(coreLib EXPORT_FILE_NAME "coreLibExport.h")

add_custom_command(TARGET coreLib PRE_BUILD
  COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different 
	"${CMAKE_CURRENT_BINARY_DIR}/coreLibExport.h" 
	"${CMAKE_BINARY_DIR}"
  COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different 
	"${CMAKE_CURRENT_BINARY_DIR}/coreLibExport.h" 
	"${CMAKE_BINARY_DIR}/coreLibExport"
)

target_link_libraries(coreLib
	Qt5::Core
	Qt5::Widgets
	Qt5::Network
	qca
	pluginManager
)

# Visual Studio structure
set_property(TARGET coreLib 
	PROPERTY FOLDER "Libraries")
source_group("Sources Files" FILES ${coredll_SRCS} ${coredll_HDRS})
source_group("Backends\\Source Files" FILES ${coredll_be_SRCS} ${coredll_be_HDRS})
source_group("Backends\\Lan\\Source Files" FILES ${coredll_be_lan_SRCS} ${coredll_be_lan_HDRS})
source_group("Backends\\Loopback\\Source Files" FILES ${coredll_be_loop_SRCS} ${coredll_be_loop_HDRS})
