file(GLOB_RECURSE SRC_FILES *.c)

add_library(jplatform STATIC ${SRC_FILES})

target_include_directories(jplatform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
