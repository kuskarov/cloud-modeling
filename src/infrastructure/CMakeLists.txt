set(SOURCES
        resource.h
        resource.cpp
        server.h
        server.cpp
        data-center.h
        vm.h
        vm.cpp
        cloud.h
        vm-storage.h
        vm-storage.cpp)

add_library(infrastructure STATIC ${SOURCES})

target_include_directories(infrastructure PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(infrastructure PUBLIC
        util
        events)
