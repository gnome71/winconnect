project(kdeconnectcore)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

#add_subdirectory(backends/loopback)

set(SOURCES
#	${backends_SOURCES}

#	backends/linkprovider.cpp
#	backends/devicelink.cpp
#	backends/pairinghandler.cpp

	kclogger.cpp
	kdeconnectconfig.cpp
	deviceidhelper.cpp
	networkpackage.cpp
	udplistener.cpp
#	daemon.cpp
#	device.cpp
	)

set(HEADERS
	kclogger.h
	kdeconnectconfig.h
	deviceidhelper.h
	networkpackage.h
#	daemon.h
#	device.h
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
