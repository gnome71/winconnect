cmake_minimum_required(VERSION 3.0)
project(sharedLibs)

# Export no symbols by default (if the compiler supports it).
# This makes e.g. GCC's "visibility behavior" consistent with MSVC's. 
# On Windows/MSVC this is a noop.
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Always include the source and build directories in the include path.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(lib)

# create the executable
add_executable(main main.cpp)

# link our executable "main" with the shared library "shared"
target_link_libraries(main shared)

install(TARGETS main
	RUNTIME DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin
	LIBRARY DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/lib
	ARCHIVE DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/lib
)

