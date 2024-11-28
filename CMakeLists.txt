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

    add_compile_options(/wd4018) # signed/unsigned mismatch
    add_compile_options(/wd4200) # nonstandard extension used: zero-sized array in struct/union
    add_compile_options(/wd4244) # conversion from 'type1' to 'type2', possible loss of data
    add_compile_options(/wd4267) # conversion from 'size_t' to 'type', possible loss of data
    add_compile_options(/wd4996) # deprecated stuff in utf8cpp library

    add_compile_options(/D_CRT_SECURE_NO_WARNINGS)

    # it's needed for some reason
    add_definitions(-DWINDOWS_IGNORE_PACKING_MISMATCH)
endif()

set(CMAKE_SYSTEM_PROCESSOR x86)

add_subdirectory(ja2lib)
add_subdirectory(platform-dummy)

if(WIN32)
    add_subdirectory(platform-win32)
    add_subdirectory(bin-win32)
else()
    add_subdirectory(platform-linux)
endif()

add_subdirectory(libs/googletest)
add_subdirectory(unittester)
