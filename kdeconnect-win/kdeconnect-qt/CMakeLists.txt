project(winconnect-qt)

set(KDECONNECT_VERSION_MAJOR 0)
set(KDECONNECT_VERSION_MINOR 2)
set(KDECONNECT_VERSION_PATCH 1)
set(KDECONNECT_VERSION "${KDECONNECT_VERSION_MAJOR}.${KDECONNECT_VERSION_MINOR}.${KDECONNECT_VERSION_PATCH}")

cmake_minimum_required(VERSION 3.2.2)

### Macros, functions, ...
macro(copy_deploy qt5core_dir)
	get_filename_component(qt_dir "${qt5core_dir}/../../../" REALPATH)

	add_custom_target(copy_deploy_files ALL
		COMMENT "Copying deploy files"
	)
		add_custom_command(TARGET copy_deploy_files POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory
				"${CMAKE_BINARY_DIR}/plugins/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>"
				"$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins"
			COMMAND ${CMAKE_COMMAND} -E copy_directory
				"${qt_dir}/$<$<CONFIG:Debug>:debug/>bin/Qca/crypto"
				"$<TARGET_FILE_DIR:${PROJECT_NAME}>/crypto"
			COMMENT "Copying folders"
		)
endmacro(copy_deploy)

macro(windeploy qt5core_dir)
	get_filename_component(qt_dir "${qt5core_dir}/../../../" REALPATH)
	set(windeploy_dir "${qt_dir}/tools")
	set(buildtype $<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>)
	
	add_custom_target(run_windeployqt 
		COMMENT "Run windeployqt"
	)

	if(${buildtype} MATCHES "Debug")
		file(WRITE "${windeploy_dir}/qt.conf" "[Paths]\nPrefix = ${qt_dir}/debug")
	endif()

	set(execute_command ${windeploy_dir}/windeployqt.exe --debug --compiler-runtime -network --list mapping ${CMAKE_BINARY_DIR}/Debug/${PROJECT_NAME}.exe)
	execute_process(
		COMMAND ${execute_command}
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Debug
		OUTPUT_VARIABLE files
		TIMEOUT 5
	)
	foreach(file ${files})
		if(NOT ${file} MATCHES "^Warning:")
			string(REPLACE " " ";" fl ${file})
			list(GET fl 0 in)
			list(GET fl 1 out)
			message(STATUS Copying file: ${in} to ${out})
			add_custom_command(TARGET run_windeployqt POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different
					${in}
					${CMAKE_BINARY_DIR}/${out}
			)
		endif(NOT ${file})
	endforeach(file)

	file(REMOVE "${windeploy_dir}/qt.conf")
endmacro(windeploy)
### End macros, functions, ...

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
set(SOURCES
    main.cpp
    mainwindow.cpp
	interfaces/devicesmodel.cpp
	interfaces/notificationinterface.cpp
	)

set(HEADERS
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
)

include(GenerateExportHeader)

# Build the project (headers included here for IDEs)
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${FORMS} ${RESOURCES})

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
source_group("Source Files" FILES ${SOURCES} ${HEADERS})
source_group("Misc Files" FILES ${MISC})
#source_group("Core\\Header Files" FILES ${core_HDRS})
#source_group("Core\\Source Files" FILES ${core_SRCS})
  
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS_DEBUG "_CONSOLE")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "_CONSOLE")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")

# Install targets to deploy dir
get_filename_component(_PKG_BASE ${Qca_lib} DIRECTORY)
set(PKG_BASE ${_PKG_BASE})

set(DEPLOY_FILES
	${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
	${SSL_EAY_RELEASE}
	${LIB_EAY_RELEASE}
	${Qca_lib}
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

# Copy plugin and crypto folder
get_filename_component(qt_dir "${Qt5Core_DIR}/../../../" REALPATH)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_BINARY_DIR}/plugins/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>"
		"$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins"
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${qt_dir}/$<$<CONFIG:Debug>:debug/>bin/Qca/crypto"
		"$<TARGET_FILE_DIR:${PROJECT_NAME}>/crypto"
	COMMENT "Copying folders"
)

# Run windeployqt
add_custom_target(run_windeployqt
	COMMENT "Run windeployqt"
)
set(windeploy_dir "${qt_dir}/tools")
set(buildtype $<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>)
	
# TODO
if($<CONFIG:Debug>)
	file(WRITE "${windeploy_dir}/qt.conf" "[Paths]\nPrefix = ${qt_dir}/debug")
endif(${buildtype})

set(execute_command ${windeploy_dir}/windeployqt.exe --debug --compiler-runtime -network --list mapping ${CMAKE_BINARY_DIR}/Debug/${PROJECT_NAME}.exe)
execute_process(
	COMMAND ${execute_command}
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Debug
	OUTPUT_VARIABLE files
	TIMEOUT 5
)
foreach(file ${files})
	if(NOT ${file} MATCHES "^Warning:")
		string(REPLACE " " ";" fl ${file})
		list(GET fl 0 in)
		list(GET fl 1 out)
		if(${out})
			add_custom_command(TARGET ${run_windeployqt} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different
					${in}
					${CMAKE_BINARY_DIR}/${out}
			)
		endif(${out})
	endif(NOT ${file})
endforeach(file)

#file(REMOVE "${windeploy_dir}/qt.conf")
