add_subdirectory(yaml-cpp)
add_subdirectory(argparse)

add_library(loguru
        loguru/loguru.hpp
        loguru/loguru.cpp)

target_compile_features(loguru PUBLIC cxx_std_17)
target_compile_definitions(loguru PUBLIC LOGURU_STACKTRACES=0)

target_include_directories(loguru PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/loguru)

target_link_libraries(loguru PUBLIC
        pthread
        dl)
