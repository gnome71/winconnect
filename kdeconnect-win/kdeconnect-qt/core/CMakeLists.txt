set(SOURCES
	kclogger.cpp
	kdeconnectconfig.cpp
	deviceidhelper.cpp
)

set(HEADERS
	kclogger.h
	kdeconnectconfig.h
	deviceidhelper.h
)

# Allow the developer to select if Dynamic or Static libraries are built
OPTION (BUILD_SHARED_LIBS "Build Shared Libraries" ON)
# Set the LIB_TYPE variable to STATIC
SET (LIB_TYPE STATIC)
IF (BUILD_SHARED_LIBS)
  # User wants to build Dynamic Libraries, so change the LIB_TYPE variable to CMake keyword 'SHARED'
  SET (LIB_TYPE SHARED)
ENDIF (BUILD_SHARED_LIBS)

# Create a target for the library
add_library(kdeconnectcore ${LIB_TYPE} ${SOURCES})
target_link_libraries(kdeconnectcore
PUBLIC
	Qt5::Network
	qca-qt5
)

GENERATE_EXPORT_HEADER( kdeconnectcore
#			 BASE_NAME KDECONNECTCORE
#			 EXPORT_MACRO_NAME KDECONNECTCORE_EXPORT
			 EXPORT_FILE_NAME kdeconnectcore_Export.h
#			 STATIC_DEFINE KDECONNECTCORE_BUILT_AS_STATIC
)

install(TARGETS kdeconnectcore
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)
