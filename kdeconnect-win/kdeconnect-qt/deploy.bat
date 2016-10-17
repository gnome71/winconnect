@set VS2015_AMD64="D:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\amd64\vcvars64.bat"
@set WDQT="D:\Qt\5.7\msvc2015_64\bin\windeployqt.exe"
@set DPDIR="D:\Users\Developer\Projekte\git\kdeconnect\kdeconnect-win\kdeconnect-qt\_deploy64bit"

@cd %DPDIR%
@call %VS2015_AMD64%
call %WDQT% kdeconnect-qt.exe
