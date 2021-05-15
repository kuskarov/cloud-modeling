add_executable(client client.cpp rpc-client.h rpc-client.cpp cli.h cli.cpp)

target_compile_features(client PUBLIC cxx_std_17)

target_link_libraries(client PUBLIC
        util
        protocol
        replxx::replxx
        argparse::argparse)
