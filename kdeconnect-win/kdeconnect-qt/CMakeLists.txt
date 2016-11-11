project(winconnect-qt)

set(KDECONNECT_VERSION_MAJOR 0)
set(KDECONNECT_VERSION_MINOR 2)
set(KDECONNECT_VERSION_PATCH 0)
set(KDECONNECT_VERSION "${KDECONNECT_VERSION_MAJOR}.${KDECONNECT_VERSION_MINOR}.${KDECONNECT_VERSION_PATCH}")

cmake_minimum_required(VERSION 3.2.2)

# Set policies
#cmake_policy(SET CMP0020 NEW)
#cmake_policy(SET CMP0003 OLD)

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
#	get_target_property(Qca-qt5_location qca-qt5 LOCATION)
#	message(STATUS "Qca-qt5 location: " ${Qca-qt5_location})
endif(Qca-qt5_FOUND)
include_directories(${Qca-qt5_INCLUDES})
add_definitions(${Qca-qt5_DEFINITIONS})

include(GenerateExportHeader)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
)
configure_file(winconnect-version.h.in ${CMAKE_CURRENT_BINARY_DIR}/winconnect-version.h)

# Include subdirectories
add_subdirectory(core)
add_subdirectory(plugins)

# Set files
set(SOURCES
	${core_SRCS}

    main.cpp
    mainwindow.cpp
	core/backends/loopback/loopbacklinkprovider.cpp
	core/backends/loopback/loopbackdevicelink.cpp
	interfaces/devicesmodel.cpp
	interfaces/notificationinterface.cpp
	#plugins/ping/pingplugin.cpp
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

include(GenerateExportHeader)

# Build the project
add_executable(${PROJECT_NAME} ${SOURCES} ${RESOURCES})
# Linker commands
target_link_libraries(${PROJECT_NAME}
	Qt5::Core
	Qt5::Gui
	Qt5::Widgets
	Qt5::Network
	Qt5::Concurrent
	qca-qt5
	pluginManager)

# Install targets to deploy dir
install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)
install(DIRECTORY lib/crypto/ DESTINATION bin/crypto)
install(FILES ${DEPLOY_FILES} DESTINATION bin)
# Install targets to binary dir
install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
	LIBRARY DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/lib
	ARCHIVE DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/lib
)
install(DIRECTORY lib/crypto/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/crypto)
install(FILES ${DEPLOY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Show all files in qtcreator projects
add_custom_target(forms EXCLUDE_FROM_ALL SOURCES ${FORMS})
