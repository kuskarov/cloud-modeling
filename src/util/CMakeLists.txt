add_library(util INTERFACE)

target_include_directories(util INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(util INTERFACE cxx_std_17)

target_link_libraries(util INTERFACE
        fmt::fmt
        NamedType)
