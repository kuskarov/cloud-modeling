set(SOURCES
        types.h
        types.cpp
        file-utils.h
        logger.h)

add_library(util STATIC ${SOURCES})

target_include_directories(util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(util PUBLIC cxx_std_17)

target_link_libraries(util PUBLIC fmt::fmt)
