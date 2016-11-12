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
find_package(Qt5Core REQUIRED)
if(Qt5Core_FOUND)
	message(STATUS "Qt5Core found v" ${Qt5Core_VERSION})
endif(Qt5Core_FOUND)
include_directories(${Qt5Core_INCLUDES})
add_definitions(${Qt5Core_DEFINITIONS})

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

find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
	message(STATUS "OpenSSL found v" ${OPENSSL_VERSION})
endif(OpenSSL_FOUND)

find_package(Qca-qt5 REQUIRED CONFIG)
if(Qca-qt5_FOUND)
    message(STATUS "Qca-qt5 found v" ${Qca-qt5_VERSION})
	get_target_property(Qca-qt5_lib qca-qt5 LOCATION)
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
get_filename_component(_PKG_BASE ${Qca-qt5_lib} DIRECTORY)
set(PKG_BASE ${_PKG_BASE}/..)
set(DEPLOY_FILES
	${PKG_BASE}/bin/libeay32.dll
	${PKG_BASE}/bin/ssleay32.dll
	${Qca-qt5_lib}
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
	Qt5::WinMain
	qca-qt5
	pluginManager
	crypt32
	ws2_32
)

# VSConfig specific settings (show or hide console)
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")

# Install targets to deploy dir
install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)
install(DIRECTORY ${PKG_BASE}/lib/qca-qt5/crypto/ DESTINATION bin/crypto)
install(FILES
	${DEPLOY_FILES}
	${CMAKE_BINARY_DIR}/pluginManager.dll
	DESTINATION bin
)
install(FILES
	${CMAKE_BINARY_DIR}/plugins/testPluginA.dll
	DESTINATION bin/plugins
)

# Copy ssl and qca libs to bin dir
foreach(lib ${DEPLOY_FILES})
	add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
		COMMAND ${CMAKE_COMMAND} ARGS -E copy
			"${lib}"
			"${CMAKE_BINARY_DIR}"
			COMMENT "Copying ${lib} to binary dir"
	)
endforeach(lib)
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
	COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory
		"${PKG_BASE}/lib/qca-qt5/crypto"
		"$<TARGET_FILE_DIR:${PROJECT_NAME}>/crypto"
		COMMENT "Copying ${PKG_BASE}/lib/qca-qt5/crypto folder to binary dir"
)

# Run windeployqt.exe in binary dir
get_filename_component(_WINDEPLOY_DIR ${QT_QMAKE_EXECUTABLE} DIRECTORY)
add_custom_target(windeploy-build-dir
	"${_WINDEPLOY_DIR}/windeployqt.exe"	"${CMAKE_BINARY_DIR}/${PROJECT_NAME}.exe"
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	COMMENT "Running windeployqt in " ${CMAKE_BINARY_DIR}
)
# Run windeployqt.exe in install prefix dir
add_custom_target(windeploy-deploy-dir
	"${_WINDEPLOY_DIR}/windeployqt.exe"	"${CMAKE_INSTALL_PREFIX}/bin/${PROJECT_NAME}.exe"
	WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin
	COMMENT "Running windeployqt in " ${CMAKE_INSTALL_PREFIX}/bin
)

# Show all files in qtcreator projects
add_custom_target(forms EXCLUDE_FROM_ALL SOURCES ${FORMS} plugins/test/testPluginA.json)
