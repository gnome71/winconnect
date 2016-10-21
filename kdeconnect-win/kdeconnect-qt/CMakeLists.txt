project(kdeconnect-qt)

set(KDECONNECT_VERSION_MAJOR 0)
set(KDECONNECT_VERSION_MINOR 1)
set(KDECONNECT_VERSION_PATCH 0)
set(KDECONNECT_VERSION "${KDECONNECT_VERSION_MAJOR}.${KDECONNECT_VERSION_MINOR}.${KDECONNECT_VERSION_PATCH}")

cmake_minimum_required(VERSION 2.8.12)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xclang -Wno-microsoft-enum-value")
endif()

# Instruct CMake to run moc/uic automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
# As moc files are generated in the binary dir:
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Find Qca-qt5 in build tree
set(Qca-qt5_DIR ${PROJECT_SOURCE_DIR}/lib/qca/qca64-debug-dll/libd/cmake/Qca-qt5)

# Search needed libs
find_package(Qt5Widgets REQUIRED)
if(Qt5Widgets_FOUND)
	message(STATUS "Qt5Widgets found v" ${Qt5Widgets_VERSION})
endif(Qt5Widgets_FOUND)
include_directories(${Qt5Widgets_INCLUDES})
add_definitions(${Qt5Widgets_DEFINITIONS})

find_package(Qt5Network REQUIRED)
if(Qt5Network_FOUND)
	message(STATUS "Qt5Network found v" ${Qt5Network_VERSION})
endif(Qt5Network_FOUND)
include_directories(${Qt5Network_INCLUDES})
add_definitions(${Qt5Network_DEFINITIONS})

find_package(Qt5Concurrent REQUIRED)
if(Qt5Concurrent_FOUND)
	message(STATUS "Qt5Concurrent found v" ${Qt5Concurrent_VERSION})
endif(Qt5Concurrent_FOUND)
include_directories(${Qt5Concurrent_INCLUDES})
add_definitions(${Qt5Concurrent_DEFINITIONS})

find_package(Qca-qt5 REQUIRED CONFIG)
if(Qca-qt5_FOUND)
    message(STATUS "Qca-qt5 found v" ${Qca-qt5_VERSION})
endif(Qca-qt5_FOUND)
include_directories(${Qca-qt5_INCLUDES})
add_definitions(${Qca-qt5_DEFINITIONS})

include(GenerateExportHeader)
include_directories(${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
)
configure_file(kdeconnect-version.h.in ${CMAKE_CURRENT_BINARY_DIR}/kdeconnect-version.h)

# Set files
set(SOURCES
    main.cpp
    mainwindow.cpp
	core/kclogger.cpp
	core/device.cpp
	core/deviceidhelper.cpp
	core/kdeconnectconfig.cpp
	core/networkpackage.cpp
	core/udplistener.cpp
	core/backends/linkprovider.cpp
	core/backends/devicelink.cpp
	core/backends/pairinghandler.cpp
	)
set(HEADERS
	dlldefines.h
    mainwindow.h
	core/kclogger.h
	core/deviceidhelper.h
	core/kdeconnectconfig.h
	core/networkpackage.h
	core/networkpackagetypes.h
	core/udplistener.h
	#	core/daemon.h
#	core/device.h
)

set(FORMS
    mainwindow.ui
)
set(RESOURCES
	res/resources.qrc
)
set(DEPLOY_FILES
	lib/libeay32.dll
	lib/ssleay32.dll
	lib/qca-qt5d.dll
)
# Include subdirectories
add_subdirectory(core)

# Build the project
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${RESOURCES})
# Linker commands
target_link_libraries(${PROJECT_NAME}
	Qt5::Core
	Qt5::Gui
	Qt5::Widgets
	Qt5::Network
	Qt5::Concurrent
	qca-qt5)

# Install targets to deploy dir
install(TARGETS kdeconnect-qt
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)
install(DIRECTORY lib/crypto/ DESTINATION bin/crypto)
install(FILES ${DEPLOY_FILES} DESTINATION bin)
# Install targets to binary dir
install(TARGETS kdeconnect-qt
	RUNTIME DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
	LIBRARY DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/lib
	ARCHIVE DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/lib
)
install(DIRECTORY lib/crypto/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/crypto)
install(FILES ${DEPLOY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


# Show all files in qtcreator projects
add_custom_target(forms EXCLUDE_FROM_ALL SOURCES ${FORMS})
