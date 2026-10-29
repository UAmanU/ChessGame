cmake_minimum_required(VERSION 3.20)
project(ChessGame VERSION 1.0 LANGUAGES CXX)
include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(RAYLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../Libraries/raylib_files" CACHE PATH "Raylib library directory")
find_path(RAYLIB_INCLUDE_DIR raylib.h PATHS "${RAYLIB_ROOT}" NO_DEFAULT_PATH REQUIRED)
find_library(RAYLIB_LIBRARY NAMES raylib libraylib.a PATHS "${RAYLIB_ROOT}" NO_DEFAULT_PATH REQUIRED)

file(GLOB_RECURSE CHESS_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_executable(ChessGame ${CHESS_SOURCES})
target_include_directories(ChessGame PRIVATE include "${RAYLIB_INCLUDE_DIR}")
target_link_libraries(ChessGame PRIVATE "${RAYLIB_LIBRARY}" winmm gdi32 opengl32)
target_compile_options(ChessGame PRIVATE -Wall -Wextra -Wpedantic)
set_target_properties(ChessGame PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

if(BUILD_TESTING)
    add_executable(ChessLogicTests test/chess_logic_tests.cpp src/chess/board.cpp src/chess/engine.cpp)
    target_include_directories(ChessLogicTests PRIVATE include)
    add_test(NAME ChessLogicTests COMMAND ChessLogicTests)
endif()
