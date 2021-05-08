add_executable(simulator simulator.cpp)

target_compile_features(simulator PUBLIC cxx_std_17)
target_compile_options(simulator PUBLIC -Wall)#-Wextra -Werror)
target_link_libraries(simulator PUBLIC
        util
        events
        infrastructure
        core)
