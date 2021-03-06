project(winconnect-qt)

set(KDECONNECT_VERSION_MAJOR 0)
set(KDECONNECT_VERSION_MINOR 2)
set(KDECONNECT_VERSION_PATCH 1)
set(KDECONNECT_VERSION "${KDECONNECT_VERSION_MAJOR}.${KDECONNECT_VERSION_MINOR}.${KDECONNECT_VERSION_PATCH}")

cmake_minimum_required(VERSION 3.2.2)

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
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
# For VS Solution Explorer
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_INSTALL_DEBUG_LIBRARIES ON)
include(InstallRequiredSystemLibraries)

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

find_package(Qca REQUIRED)
if(Qca_FOUND)
    message(STATUS "Qca found v" ${Qca_VERSION})
	get_target_property(Qca_lib qca LOCATION)
	get_target_property(Qca_inc qca INTERFACE_INCLUDE_DIRECTORIES)
endif(Qca_FOUND)
include_directories(${Qca_inc})
add_definitions(${Qca_DEFINITIONS})

include(GenerateExportHeader)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
)
configure_file(winconnect-version.h.in ${CMAKE_CURRENT_BINARY_DIR}/winconnect-version.h)

# Include subdirectories
add_subdirectory(core)
add_subdirectory(plugins)

# Set files
set(SRCS
    main.cpp
    mainwindow.cpp
	interfaces/devicesmodel.cpp
	interfaces/notificationinterface.cpp
	)

set(HDRS
    mainwindow.h
	interfaces/devicesmodel.h
	interfaces/notificationinterface.h
)

set(FORMS
	mainwindow.ui
)

set(RESOURCES
	res/resources.qrc
)

set(MISC
    mainwindow.ui
	res/resources.qrc
	vstools/qt5.natvis
	vstools/winconnect.natvis
)

include(GenerateExportHeader)

# Build the project (headers included here for IDEs)
add_executable(${PROJECT_NAME} ${SRCS} ${HDRS} ${FORMS} ${RESOURCES} ${MISC})

# Linker commands
target_link_libraries(${PROJECT_NAME}
	Qt5::Core
	Qt5::Gui
	Qt5::Widgets
	Qt5::Network
	Qt5::Concurrent
	Qt5::WinMain
	qca
	pluginManager
	coreLib
)

# VSConfig specific settings (show or hide console, ...)
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "Binaries")
source_group("Source Files" FILES ${SRCS} ${HDRS})
source_group("Misc Files" FILES ${MISC})
  
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")

# TODO: Install targets to deploy dir
get_filename_component(_PKG_BASE ${Qca_lib} DIRECTORY)
set(PKG_BASE ${_PKG_BASE})

set(DEPLOY_FILES
	${SSL_EAY_RELEASE}
	${LIB_EAY_RELEASE}
)

install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)
install(DIRECTORY ${PKG_BASE}/Qca/crypto/ DESTINATION bin/crypto)
install(FILES
	${DEPLOY_FILES}
	${CMAKE_BINARY_DIR}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/pluginManager.dll
	DESTINATION bin
)
install(FILES
	${CMAKE_BINARY_DIR}/plugins/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/testPluginA.dll
	DESTINATION bin/plugins
)

# Setting stuff for post build
get_filename_component(QT_DIR "${Qt5Core_DIR}/../../../" REALPATH CACHE)
get_target_property(qmake_exe ${Qt5Core_QMAKE_EXECUTABLE} LOCATION)
get_filename_component(WINDEPLOYQT_DIR ${qmake_exe} DIRECTORY CACHE)
set(WINDEPLOYQT_EXE ${WINDEPLOYQT_DIR}/windeployqt.exe CACHE FILEPATH "windeployqt executable")

# The command to copy all the needed files and folders
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_BINARY_DIR}/plugins/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>"
		"$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins"
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${QT_DIR}/$<$<CONFIG:Debug>:debug/>bin/Qca/crypto"
		"$<TARGET_FILE_DIR:${PROJECT_NAME}>/crypto"
	COMMAND ${CMAKE_COMMAND} -E copy 
		${QT_DIR}/$<$<CONFIG:Debug>:debug/>bin/ssleay32.dll 
		${QT_DIR}/$<$<CONFIG:Debug>:debug/>bin/libeay32.dll 
		"$<TARGET_FILE_DIR:${PROJECT_NAME}>"
	COMMAND ${CMAKE_COMMAND} -E echo "Build type: " $<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>
	COMMAND ${CMAKE_COMMAND} -E echo "[Paths]" > ${WINDEPLOYQT_DIR}/qt.conf
	COMMAND ${CMAKE_COMMAND} -E echo "Prefix = ${QT_DIR}$<$<CONFIG:Debug>:/debug>" >> ${WINDEPLOYQT_DIR}/qt.conf
	COMMAND ${WINDEPLOYQT_EXE} $<$<CONFIG:Debug>:--pdb> $<$<CONFIG:Debug>:--debug>$<$<CONFIG:Release>:--release> -network $<TARGET_FILE_DIR:${PROJECT_NAME}>
	COMMAND ${CMAKE_COMMAND} -E remove ${WINDEPLOYQT_DIR}/qt.conf
	COMMENT "Running post build commands"
)
