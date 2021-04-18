set(SOURCES
        resource.h
        resource.cpp
        server.h
        server.cpp
        data-center.h
        data-center.cpp
        vm.h
        vm.cpp)

add_library(infrastructure STATIC ${SOURCES})

target_include_directories(infrastructure PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(infrastructure PUBLIC cxx_std_17)

target_link_libraries(infrastructure PUBLIC
        util
        events)

target_link_libraries(infrastructure PUBLIC loguru)
