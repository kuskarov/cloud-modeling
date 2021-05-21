set(SOURCES
        custom-code.h
        util.h
        cloud-schedulers/place-to-first.h
        server-schedulers/greedy.h
        )

add_library(custom STATIC ${SOURCES})

target_include_directories(custom PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(custom PUBLIC cxx_std_17)

target_link_libraries(custom PUBLIC
        util
        events
        infrastructure
        core)
