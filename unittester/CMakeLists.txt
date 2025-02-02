file(GLOB_RECURSE SRC_FILES *.cpp)

add_executable(unittester ${SRC_FILES})

target_link_libraries(unittester PRIVATE gtest gtest_main)
target_link_libraries(unittester PRIVATE ja2lib)
target_link_libraries(unittester PRIVATE platform-dummy)
target_link_libraries(unittester PRIVATE jplatform-dummy)
if(WIN32)
else()
    target_link_libraries(unittester PRIVATE platform-linux)
endif()

target_include_directories(unittester PRIVATE "../ja2lib")
target_include_directories(unittester PRIVATE "../jplatform")

# Add test to CTest
add_test(NAME unittester COMMAND unittester)
