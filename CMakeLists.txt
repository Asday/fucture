cmake_minimum_required(VERSION 3.20)
project(not-asteroids CXX)
set(CMAKE_CXX_STANDARD 26)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CUSTOMIZE_BUILD ON CACHE BOOL "" FORCE)
set(SUPPORT_FILEFORMAT_JPG ON CACHE BOOL "" FORCE)
add_subdirectory(vendor/raylib)

add_executable(main src/main.cpp)

target_link_libraries(main PRIVATE raylib)

if(MSVC)
	target_compile_options(main PRIVATE /W4 /WX /permissive- /std:c++latest)
else()
	target_compile_options(
		main PRIVATE
		-Wall
		-Wextra
		-Werror
		-Wsign-conversion
		-Wconversion
		-pedantic-errors
	)
endif()
