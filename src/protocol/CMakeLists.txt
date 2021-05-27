set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)

set (SOURCES)

function(generate_grpc_classes)
    # Proto file
    get_filename_component(api_proto "${ARGV0}.proto" ABSOLUTE)
    get_filename_component(api_proto_path "${api_proto}" PATH)

    # Generated sources
    set(api_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/${ARGV0}.pb.cc")
    set(api_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/${ARGV0}.pb.h")
    set(api_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/${ARGV0}.grpc.pb.cc")
    set(api_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/${ARGV0}.grpc.pb.h")

    add_custom_command(
            OUTPUT "${api_proto_srcs}" "${api_proto_hdrs}" "${api_grpc_srcs}" "${api_grpc_hdrs}"
            COMMAND ${_PROTOBUF_PROTOC}
            ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
            --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
            -I "${api_proto_path}" -I "${CMAKE_SOURCE_DIR}/external/grpc/third_party/protobuf/src"
            --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
            "${api_proto}"
            DEPENDS "${api_proto}")

    set (SOURCES ${SOURCES}
            ${api_proto_srcs}
            ${api_proto_hdrs}
            ${api_grpc_srcs}
            ${api_grpc_hdrs}
            PARENT_SCOPE)
endfunction()

generate_grpc_classes(api)
generate_grpc_classes(scheduler)

add_library(protocol STATIC ${SOURCES})

target_include_directories(protocol PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(protocol PUBLIC
        grpc++_reflection
        grpc++
        libprotobuf)
