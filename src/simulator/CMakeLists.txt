add_executable(simulator simulator.cpp)

target_compile_options(simulator PUBLIC -Wall -Wextra)
target_link_libraries(simulator PUBLIC
        util
        events
        infrastructure
        protocol
        core
        custom)
