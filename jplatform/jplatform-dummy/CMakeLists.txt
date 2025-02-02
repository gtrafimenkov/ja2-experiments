file(GLOB_RECURSE SRC_FILES *.c)

add_library(jplatform-dummy STATIC ${SRC_FILES})

target_include_directories(jplatform-dummy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(jplatform-dummy PRIVATE "..")
