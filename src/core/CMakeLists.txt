set(SOURCES
        manager.h
        manager.cpp
        config.h
        config.cpp
        vm-storage.h
        vm-storage.cpp
        scheduler.h
        scheduler.cpp)

add_library(core STATIC ${SOURCES})

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(core PUBLIC cxx_std_17)

target_link_libraries(core PUBLIC
        util
        events
        infrastructure)

target_link_libraries(core PUBLIC
        yaml-cpp::yaml-cpp
        argparse::argparse)
