project(winconnect-qt)

set(KDECONNECT_VERSION_MAJOR 0)
set(KDECONNECT_VERSION_MINOR 2)
set(KDECONNECT_VERSION_PATCH 1)
set(KDECONNECT_VERSION "${KDECONNECT_VERSION_MAJOR}.${KDECONNECT_VERSION_MINOR}.${KDECONNECT_VERSION_PATCH}")

cmake_minimum_required(VERSION 3.2.2)

# Instruct CMake to run moc/uic automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
# As moc files are generated in the binary dir:
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(BUILD_SHARED_LIBS ON)

# For VS Solution Explorer
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_INSTALL_DEBUG_LIBRARIES ON)
include(InstallRequiredSystemLibraries)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

# Search needed libs
find_package(Qt5Core REQUIRED)
if(Qt5Core_FOUND)
	message(STATUS "Qt5Core found v" ${Qt5Core_VERSION})
	get_target_property(QtCore_location Qt5::Core LOCATION)
	include_directories(${Qt5Core_INCLUDE_DIRS})
	add_definitions(${Qt5Core_DEFINITIONS})
	#message(STATUS ${Qt5Core_INCLUDE_DIRS} | ${Qt5Core_DEFINITIONS})
endif(Qt5Core_FOUND)

find_package(Qt5Widgets REQUIRED)
if(Qt5Widgets_FOUND)
	message(STATUS "Qt5Widgets found v" ${Qt5Widgets_VERSION})
	get_target_property(QtWidgets_location Qt5::Widgets LOCATION)
	include_directories(${Qt5Widgets_INCLUDE_DIRS})
	add_definitions(${Qt5Widgets_DEFINITIONS})
	#message(STATUS ${Qt5Widgets_INCLUDE_DIRS} | ${Qt5Widgets_DEFINITIONS})
endif(Qt5Widgets_FOUND)

find_package(Qt5Network REQUIRED)
if(Qt5Network_FOUND)
	message(STATUS "Qt5Network found v" ${Qt5Network_VERSION})
	get_target_property(QtNetwork_location Qt5::Network LOCATION)
	include_directories(${Qt5Network_INCLUDE_DIRS})
	add_definitions(${Qt5Network_DEFINITIONS})
	#message(STATUS ${Qt5Network_INCLUDE_DIRS} | ${Qt5Network_DEFINITIONS})
endif(Qt5Network_FOUND)

find_package(Qt5Concurrent REQUIRED)
if(Qt5Concurrent_FOUND)
	message(STATUS "Qt5Concurrent found v" ${Qt5Concurrent_VERSION})
	get_target_property(QtConcurrent_location Qt5::Concurrent LOCATION)
	include_directories(${Qt5Concurrent_INCLUDE_DIRS})
	add_definitions(${Qt5Concurrent_DEFINITIONS})
	#message(STATUS ${Qt5Concurrent_INCLUDE_DIRS} | ${Qt5Concurrent_DEFINITIONS})
endif(Qt5Concurrent_FOUND)

find_package(OpenSSL)
if(OpenSSL_FOUND)
	message(STATUS "OpenSSL found v" ${OPENSSL_VERSION})
endif(OpenSSL_FOUND)

find_package(Qca REQUIRED)
if(Qca_FOUND)
    message(STATUS "Qca found v" ${Qca_VERSION})
	get_target_property(Qca_location qca LOCATION)
	get_target_property(Qca_INCLUDE_DIRS qca INTERFACE_INCLUDE_DIRECTORIES)
	include_directories(${Qca_INCLUDE_DIRS})
	#message(STATUS ${Qca_INCLUDE_DIRS})
endif(Qca_FOUND)

include(GenerateExportHeader)

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

# Build the project (headers included here for IDEs)
add_executable(${PROJECT_NAME} ${SRCS} ${HDRS} ${FORMS} ${RESOURCES} ${MISC})

# Linker commands
target_link_libraries(${PROJECT_NAME}
	${QtCore_location}
	${QtWidgets_location}
	${QtNetwork_location}
	${QtConcurrent_location}
	${Qca_location}
#	pluginManager
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
#get_filename_component(_PKG_BASE ${Qca_lib} DIRECTORY)
#set(PKG_BASE ${_PKG_BASE})

#set(DEPLOY_FILES
#	${SSL_EAY_RELEASE}
#	${LIB_EAY_RELEASE}
#)

#install(TARGETS ${PROJECT_NAME}
#	RUNTIME DESTINATION bin
#	LIBRARY DESTINATION lib
#	ARCHIVE DESTINATION lib
#)
#install(DIRECTORY ${PKG_BASE}/Qca/crypto/ DESTINATION bin/crypto)
#install(FILES
#	${DEPLOY_FILES}
#	${CMAKE_BINARY_DIR}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/pluginManager.dll
#	DESTINATION bin
#)
#install(FILES
#	${CMAKE_BINARY_DIR}/plugins/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/testPluginA.dll
#	DESTINATION bin/plugins
#)

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
