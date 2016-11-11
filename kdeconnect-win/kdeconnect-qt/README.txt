Building Qca-qt5 for windows:

install vcpkg according to:
	https://github.com/Microsoft/vcpkg/blob/master/docs/EXAMPLES.md

install openssl:
	vcpkg install openssl:x64-windows

compile qca release:
	add: CMAKE_INSTALL_PREFIX=d:\src\vcpkg\installed\x64-windows
	add: CMAKE_TOOLCHAIN_FILE=d:\src\vcpkg\scripts\buildsystems\vcpkg.cmake
	shared_libs on
	tools off
	tests off
	add install build target
	build

compile qca debug:
	add: CMAKE_INSTALL_PREFIX=d:\src\vcpkg\installed\x64-windows\debug
	add: CMAKE_TOOLCHAIN_FILE=d:\src\vcpkg\scripts\buildsystems\vcpkg.cmake
	shared_libs on
	tools off
	tests off
	add install build target
	build

