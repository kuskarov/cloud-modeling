add_subdirectory(yaml-cpp)
add_subdirectory(argparse)
add_subdirectory(fmt)
add_subdirectory(replxx)
add_subdirectory(NamedType)

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
add_subdirectory(grpc)
