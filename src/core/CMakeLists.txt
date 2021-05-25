set(SOURCES
        world.h
        world.cpp
        config.h
        config.cpp
        scheduler.h
        resource-scheduler.h
        rpc-service.h
        rpc-service.cpp
        rpc-scheduler.h)

add_library(core STATIC ${SOURCES})

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(core PUBLIC
        util
        events
        infrastructure
        custom
        protocol)

target_link_libraries(core PUBLIC
        yaml-cpp::yaml-cpp
        argparse::argparse)
