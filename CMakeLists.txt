cmake_minimum_required(VERSION 3.10)

enable_testing()

if(WIN32)
    add_compile_options(/utf-8)
endif()

# Using the same debug library as google test.
# Otherwise it will not link.
if(WIN32)
  # https://cmake.org/cmake/help/latest/variable/CMAKE_MSVC_RUNTIME_LIBRARY.html
  # https://stackoverflow.com/questions/10113017/setting-the-msvc-runtime-in-cmake#56776146
  cmake_policy(SET CMP0091 NEW)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

project(ja2v)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

# if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
#     # add_compile_options(-Wno-unused-parameter)  # Example for GCC
# elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#     # add_compile_options(-Wno-unused-parameter)  # Example for Clang
# endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    message(STATUS "Compiler: MSVC")

    # treating warnings as errors
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /WX")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")

    add_compile_options(/wd4267) # conversion from 'size_t' to 'type', possible loss of data
    # warning C4996: 'strupr': The POSIX name for this item is deprecated. Instead, use the ISO C and C++ conformant name: _strupr. See online help for details.
    add_compile_options(/wd4996)

    add_compile_options(/D_CRT_SECURE_NO_WARNINGS)

    # it's needed for some reason
    add_definitions(-DWINDOWS_IGNORE_PACKING_MISMATCH)
endif()

add_subdirectory(ja2lib)
add_subdirectory(platform-dummy)
add_subdirectory(jplatform/jplatform-dummy)

if(WIN32)
    add_subdirectory(platform-win32)
    add_subdirectory(jplatform/jplatform-windd2)
    add_subdirectory(bin-win32)
else()
    add_subdirectory(platform-linux)
endif()

add_subdirectory(libs/googletest)
add_subdirectory(unittester)
