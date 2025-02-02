file(GLOB_RECURSE SRC_FILES *.c)

add_library(jplatform-windd2 STATIC ${SRC_FILES})

target_include_directories(jplatform-windd2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(jplatform-windd2 PRIVATE "..")

# TODO: platform code should not depend on ja2lib
target_include_directories(jplatform-windd2 PRIVATE "../../ja2lib")
