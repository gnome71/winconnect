macro(run_windeployqt)
	message(STATUS "qt_dir: " ${QT_DIR})
	message(STATUS "wd: " ${WINDEPLOYQT_EXE})
	set(buildtype $<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>)
	
    if(${buildtype} MATCHES "Debug")
		file(WRITE "${WINDEPLOYQT_DIR}/qt.conf" "[Paths]\nPrefix = ${QT_DIR}/debug\nArchData = ${QT_DIR}/share/qt5/mkspecs")
		exec_program(${WINDEPLOYQT_EXE} ARGS "-network ${CMAKE_BINARY_DIR}/Debug/${PROJECT_NAME}.exe")
	elseif()
		exec_program(${WINDEPLOYQT_EXE} ARGS "-network ${CMAKE_BINARY_DIR}/Release/${PROJECT_NAME}.exe")
    endif(${buildtype})

	#file(REMOVE "${windeployqt_dir}/qt.conf")
endmacro(run_windeployqt)
