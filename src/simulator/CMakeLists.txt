add_executable(simulator simulator.cpp)

target_compile_features(simulator PUBLIC cxx_std_17)
target_link_libraries(simulator PUBLIC
        util
        events
        resources
        core)

target_link_libraries(simulator PUBLIC loguru)
