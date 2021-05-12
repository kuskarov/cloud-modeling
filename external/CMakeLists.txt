add_subdirectory(yaml-cpp)
add_subdirectory(argparse)
add_subdirectory(fmt)
add_subdirectory(replxx)

option(protobuf_BUILD_TESTS "" OFF)
add_subdirectory(grpc)
