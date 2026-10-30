cmake_minimum_required(VERSION 3.24)
project(ChessGame VERSION 1.0 LANGUAGES C CXX)

include(CTest)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Use the supplied static archive only for MinGW on Windows. It is an import-free
# static library and remains untouched; Unix and MSVC builds compile Raylib from
# its pinned source release in build/_deps instead.
set(RAYLIB_LOCAL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../Libraries/raylib_files")
if(WIN32 AND MINGW AND EXISTS "${RAYLIB_LOCAL_ROOT}/libraylib.a" AND EXISTS "${RAYLIB_LOCAL_ROOT}/raylib.h")
    add_library(raylib STATIC IMPORTED GLOBAL)
    set_target_properties(raylib PROPERTIES
        IMPORTED_LOCATION "${RAYLIB_LOCAL_ROOT}/libraylib.a"
        INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_LOCAL_ROOT}"
        INTERFACE_LINK_LIBRARIES "winmm;gdi32;opengl32"
    )
else()
    set(RAYLIB_VERSION 6.0)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(CUSTOMIZE_BUILD OFF CACHE BOOL "" FORCE)
    set(PLATFORM Desktop CACHE STRING "" FORCE)

    FetchContent_Declare(raylib
        URL "https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz"
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(raylib)
endif()

file(GLOB_RECURSE CHESS_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_executable(ChessGame ${CHESS_SOURCES})
target_include_directories(ChessGame PRIVATE include)
target_link_libraries(ChessGame PRIVATE raylib)

if(MSVC)
    target_compile_options(ChessGame PRIVATE /W4 /O2)
elseif(MINGW)
    target_compile_options(ChessGame PRIVATE -Wall -Wextra -Wpedantic -O2)
    # Bundle the MinGW C++ runtime as well as Raylib into the Windows executable.
    target_link_options(ChessGame PRIVATE -static -static-libgcc -static-libstdc++)
else()
    target_compile_options(ChessGame PRIVATE -Wall -Wextra -Wpedantic -O2)
endif()

if(APPLE)
    target_link_libraries(ChessGame PRIVATE "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
endif()

set_target_properties(ChessGame PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set_property(TARGET ChessGame PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:ChessGame>")
add_custom_command(TARGET ChessGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:ChessGame>/resources")

if(BUILD_TESTING)
    add_executable(ChessOpeningRegressionTests
        test/chess_logic_tests.cpp
        src/chess/board.cpp
        src/chess/engine.cpp
        src/chess/opening_book.cpp
    )
    target_include_directories(ChessOpeningRegressionTests PRIVATE include)
    if(MSVC)
        target_compile_options(ChessOpeningRegressionTests PRIVATE /W4 /O2)
    else()
        target_compile_options(ChessOpeningRegressionTests PRIVATE -Wall -Wextra -Wpedantic -O2)
    endif()
    add_test(NAME ChessOpeningRegressionTests COMMAND ChessOpeningRegressionTests)

    add_executable(ChessFontAssetTests test/chess_font_asset_tests.cpp)
    add_dependencies(ChessFontAssetTests ChessGame)
    target_include_directories(ChessFontAssetTests PRIVATE include)
    target_link_libraries(ChessFontAssetTests PRIVATE raylib)
    add_test(NAME ChessFontAssetTests COMMAND ChessFontAssetTests)
endif()
