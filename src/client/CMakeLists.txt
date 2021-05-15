add_executable(client client.cpp rpc-client.h rpc-client.cpp cli.h cli.cpp)

target_link_libraries(client PUBLIC
        util
        protocol
        replxx::replxx
        argparse::argparse)
