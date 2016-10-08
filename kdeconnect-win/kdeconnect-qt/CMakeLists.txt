project(kdeconnect-qt)
cmake_minimum_required(VERSION 2.8.12)

# Instruct CMake to run moc/uic automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
# As moc files are generated in the binary dir:
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# At home
set(CMAKE_PREFIX_PATH D:/Qt/5.7/msvc2015)
# At work
set(CMAKE_PREFIX_PATH F:/Qt/Qt5.7.0/5.7/msvc2015)

# Find Qca-qt5 in build tree
set(Qca-qt5_DIR ${PROJECT_SOURCE_DIR}/lib/qca/lib/cmake/Qca-qt5)

# Search needed libs
find_package(Qt5Widgets REQUIRED)
message(STATUS ${Qt5Widgets_Location})
include_directories(${Qt5Widgets_INCLUDES})
add_definitions(${Qt5Widgets_DEFINITIONS})

find_package(Qt5Network REQUIRED)
include_directories(${Qt5Network_INCLUDES})
add_definitions(${Qt5Network_DEFINITIONS})

find_package(Qt5Concurrent REQUIRED)
include_directories(${Qt5Concurrent_INCLUDES})
add_definitions(${Qt5Concurrent_DEFINITIONS})

find_package(Qca-qt5 REQUIRED CONFIG)
if(Qca-qt5_FOUND)
    message(STATUS "Qca-qt5 found v" ${Qca-qt5_VERSION})
    get_target_property(Qca_Location qca-qt5 LOCATION)
    message(STATUS "at: " ${Qca_Location})
endif(Qca-qt5_FOUND)
include_directories(${Qca-qt5_INCLUDES})
add_definitions(${Qca-qt5_DEFINITIONS})

# Set file globs
set(SOURCES
    main.cpp
    mainwindow.cpp
    core/kdeconnectconfig.cpp
    core/kclogger.cpp
)
set(HEADERS
    mainwindow.h
    core/kdeconnectconfig.h
    core/kclogger.h
)
set(FORMS
    mainwindow.ui
)

# What to do
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} res/resources.qrc)

# Show all files in project view
add_custom_target(headers SOURCES ${HEADERS})
add_custom_target(forms SOURCES ${FORMS})

# Linker commands
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    Qt5::Concurrent
    qca-qt5)
