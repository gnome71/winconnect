REM build dir
@cd %1

REM vcvars dir
@call %2

REM windeployqt dir
call %3 kdeconnect-qt.exe
