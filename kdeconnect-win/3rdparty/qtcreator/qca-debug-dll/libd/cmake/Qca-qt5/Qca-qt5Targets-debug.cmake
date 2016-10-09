#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "qca-qt5" for configuration "Debug"
set_property(TARGET qca-qt5 APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(qca-qt5 PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/libd/qca-qt5d.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/bin/qca-qt5d.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS qca-qt5 )
list(APPEND _IMPORT_CHECK_FILES_FOR_qca-qt5 "${_IMPORT_PREFIX}/libd/qca-qt5d.lib" "${_IMPORT_PREFIX}/bin/qca-qt5d.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
