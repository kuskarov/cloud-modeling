set(SOURCES
        resource.h
        server.h
        server.cpp
        data-center.h
        data-center.cpp)

add_library(resources STATIC ${SOURCES})

target_include_directories(resources PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(resources PUBLIC cxx_std_17)

target_link_libraries(resources PUBLIC
        util
        events)

target_link_libraries(resources PUBLIC loguru)
