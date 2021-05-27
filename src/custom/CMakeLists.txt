set(SOURCES
        custom-code.h
        util.h
        cloud-schedulers/place-to-first.h
        server-schedulers/greedy.h
        workload-models/constant.h
        workload-models/random-uniform.h
        )

add_library(custom STATIC ${SOURCES})

target_include_directories(custom PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(custom PUBLIC
        util
        events
        infrastructure
        core)
