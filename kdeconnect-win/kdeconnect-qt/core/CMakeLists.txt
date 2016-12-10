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

	core/kclogger.h
	core/kdeconnectconfig.h
	core/kdeconnectplugin.h
	core/kdeconnectpluginconfig.h
	core/deviceidhelper.h
	core/networkpackage.h
	core/daemon.h
	core/device.h

	PARENT_SCOPE
)


