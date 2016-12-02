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

	core/kclogger.cpp
	core/kdeconnectconfig.cpp
	core/deviceidhelper.cpp
	core/networkpackage.cpp
	core/daemon.cpp
	core/device.cpp

	PARENT_SCOPE
	)

set(core_HDRS
	${backends_HDRS}

	core/kclogger.h
	core/kdeconnectconfig.h
	core/deviceidhelper.h
	core/networkpackage.h
	core/daemon.h
	core/device.h

	PARENT_SCOPE
)

# Create a target for the library
#add_library(kdeconnectcore ${LIB_TYPE} ${SOURCES})
#target_link_libraries(kdeconnectcore
#PUBLIC
#	Qt5::Network
#	qca-qt5
#PRIVATE
#	Qt5::Gui
#)

#set_target_properties(kdeconnectcore PROPERTIES
#	VERSION ${KDECONNECT_VERSION}
#	SOVERSION ${KDECONNECT_VERSION_MAJOR}
#)

#target_include_directories(kdeconnectcore PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
#GENERATE_EXPORT_HEADER( kdeconnectcore
#			 EXPORT_FILE_NAME kdeconnectcore_export.h
#			 BASE_NAME KDEConnectCore
#)

#install(TARGETS kdeconnectcore
#	EXPORT kdeconnectLibraryTargets
#	RUNTIME DESTINATION bin
#	LIBRARY DESTINATION lib
#	ARCHIVE DESTINATION lib
#	LIBRARY NAMELINK_SKIP
#)
