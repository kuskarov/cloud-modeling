set(SOURCES
        actor.h
        event.h
        event-loop.h
        event-loop.cpp
        actor-register.h
        observer.h)

add_library(events STATIC ${SOURCES})

target_include_directories(events PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(events PUBLIC util)
