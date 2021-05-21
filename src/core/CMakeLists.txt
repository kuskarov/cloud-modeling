set(SOURCES
        world.h
        world.cpp
        config.h
        config.cpp
        scheduler.h
        resource-scheduler.h
        rpc-service.h
        rpc-service.cpp)

add_library(core STATIC ${SOURCES})

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(core PUBLIC cxx_std_17)

target_link_libraries(core PUBLIC
        util
        events
        infrastructure
        custom
        protocol)

target_link_libraries(core PUBLIC
        yaml-cpp::yaml-cpp
        argparse::argparse)
