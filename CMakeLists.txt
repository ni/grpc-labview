#----------------------------------------------------------------------
#----------------------------------------------------------------------
cmake_minimum_required(VERSION 3.15)

cmake_policy(SET CMP0091 NEW)

project(labview-grpc C CXX)
set(ABSL_ENABLE_INSTALL ON)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
  # Set default visibility to hidden, only export LIBRARY_EXPORT symbols from the shared library
  add_compile_options(-fvisibility=hidden)
else()
  add_definitions(-D_WIN32_WINNT=0x600)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "Use static runtime" FORCE)
  set(CMAKE_MSVC_RUNTIME_LIBRARY_DEFAULT "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "Default MSVC runtime library to use static runtime")
  set(gRPC_MSVC_STATIC_RUNTIME ON CACHE BOOL "Use static runtime for gRPC" FORCE)
  set(protobuf_MSVC_STATIC_RUNTIME ON CACHE BOOL "Use static runtime for protobuf" FORCE)
  set(ABSL_MSVC_STATIC_RUNTIME ON CACHE BOOL "Use static runtime for Abseil")
  set(CARES_MSVC_STATIC_RUNTIME ON CACHE BOOL "Use static runtime for c-ares")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4267 /wd4244")
  add_compile_options("$<$<NOT:$<CONFIG:Debug>>:/Zi>")
  add_link_options("$<$<NOT:$<CONFIG:Debug>>:/DEBUG>")
  add_link_options("$<$<NOT:$<CONFIG:Debug>>:/OPT:REF>")
  add_link_options("$<$<NOT:$<CONFIG:Debug>>:/OPT:ICF>")
endif()

find_package(Threads REQUIRED)

# Setup ASIO
set(ASIO_ROOT "${CMAKE_SOURCE_DIR}/third_party/asio")
set(ASIO_INCLUDE_DIRS "${ASIO_ROOT}/asio/include")

#----------------------------------------------------------------------
#----------------------------------------------------------------------
message( ${CMAKE_SIZEOF_VOID_P} )

#----------------------------------------------------------------------
#----------------------------------------------------------------------
add_definitions(-D_PS_${CMAKE_SIZEOF_VOID_P})

#----------------------------------------------------------------------
# Include the gRPC's cmake build
#----------------------------------------------------------------------
add_subdirectory(third_party/grpc ${CMAKE_CURRENT_BINARY_DIR}/grpc EXCLUDE_FROM_ALL)

#----------------------------------------------------------------------
# Use the grpc targets directly from this build.
#----------------------------------------------------------------------
set(_PROTOBUF_LIBPROTOBUF libprotobuf)
set(_REFLECTION grpc++_reflection)
if(CMAKE_CROSSCOMPILING)
  find_program(_PROTOBUF_PROTOC protoc)
else()
  set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif()
set(_GRPC_GRPCPP grpc++)
if(CMAKE_CROSSCOMPILING)
  find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
  set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
endif()

#----------------------------------------------------------------------
# Include generated *.pb.h files
#----------------------------------------------------------------------
include_directories("${CMAKE_CURRENT_BINARY_DIR}" "./src" "./third_party/grpc" "${CMAKE_CURRENT_BINARY_DIR}/gens" "${ASIO_INCLUDE_DIRS}")

#----------------------------------------------------------------------
# LabVIEW support for grpc and protobuf
#----------------------------------------------------------------------

add_custom_target(Detect_Compatibility_Breaks
    COMMAND ${CMAKE_COMMAND} -E echo "Detecting backward compatibility breakage ..."
    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeTests/run_test.py
    RESULT_VARIABLE shell_command_result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(labview_grpc_server SHARED
  src/any_support.cc
  src/cluster_copier.cc
  src/event_data.cc
  src/feature_toggles.cc
  src/grpc_client.cc
  src/grpc_interop.cc
  src/grpc_load.cc
  src/grpc_server.cc
  src/lv_interop.cc
  src/lv_message.cc
  src/lv_message_efficient.cc
  src/lv_message_value.cc
  src/lv_proto_server_reflection_plugin.cc
  src/lv_proto_server_reflection_service.cc
  src/message_element_metadata_owner.cc
  src/message_metadata.cc
  src/unpacked_fields.cc
  src/well_known_messages.cc
)
target_link_libraries(labview_grpc_server
   ${_REFLECTION}
   ${_GRPC_GRPCPP}
   ${_PROTOBUF_LIBPROTOBUF})

#----------------------------------------------------------------------
# LabVIEW code generator support library to generate client and
# server VIs from a .proto file
#----------------------------------------------------------------------
add_library(labview_grpc_generator SHARED
  src/feature_toggles.cc
  src/lv_interop.cc
  src/proto_parser.cc
  )
target_link_libraries(labview_grpc_generator
   ${_REFLECTION}
   ${_GRPC_GRPCPP}
   ${_PROTOBUF_LIBPROTOBUF})

#######################################################################
# QueryServer Example
#######################################################################

#----------------------------------------------------------------------
# Proto file
#----------------------------------------------------------------------
get_filename_component(qs_proto "examples/query_server/Protos/query_server.proto" ABSOLUTE)
get_filename_component(qs_proto_path "${qs_proto}" PATH)

#----------------------------------------------------------------------
# Generated sources
#----------------------------------------------------------------------
set(qs_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/query_server.pb.cc")
set(qs_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/query_server.pb.h")
set(qs_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/query_server.grpc.pb.cc")
set(qs_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/query_server.grpc.pb.h")
add_custom_command(
  OUTPUT "${qs_proto_srcs}" "${qs_proto_hdrs}" "${qs_grpc_srcs}" "${qs_grpc_hdrs}"
  COMMAND ${_PROTOBUF_PROTOC}
  ARGS --grpc_out="${CMAKE_CURRENT_BINARY_DIR}"
    --cpp_out="${CMAKE_CURRENT_BINARY_DIR}"
    -I="${qs_proto_path}"
    --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
    "${qs_proto}"
  DEPENDS "${qs_proto}")

#----------------------------------------------------------------------
# Example C++ application to talk to the example query server
#----------------------------------------------------------------------
add_executable(example_client
  "src/example_client.cc"
  ${qs_proto_srcs}
  ${qs_grpc_srcs}
  )
target_link_libraries(example_client
   ${_REFLECTION}
   ${_GRPC_GRPCPP}
   ${_PROTOBUF_LIBPROTOBUF})

#######################################################################
# Testing Project
#######################################################################

#----------------------------------------------------------------------
# Proto file
#----------------------------------------------------------------------
get_filename_component(dm_proto "tests/Deprecated/Protos/data_marshal.proto" ABSOLUTE)
get_filename_component(dm_proto_path "${dm_proto}" PATH)

#----------------------------------------------------------------------
# Generated sources
#----------------------------------------------------------------------
set(dm_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/data_marshal.pb.cc")
set(dm_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/data_marshal.pb.h")
set(dm_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/data_marshal.grpc.pb.cc")
set(dm_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/data_marshal.grpc.pb.h")
add_custom_command(
  OUTPUT "${dm_proto_srcs}" "${dm_proto_hdrs}" "${dm_grpc_srcs}" "${dm_grpc_hdrs}"
  COMMAND ${_PROTOBUF_PROTOC}
  ARGS --grpc_out="${CMAKE_CURRENT_BINARY_DIR}"
    --cpp_out="${CMAKE_CURRENT_BINARY_DIR}"
    -I="${dm_proto_path}"
    --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
    "${dm_proto}"
  DEPENDS "${dm_proto}")

#----------------------------------------------------------------------
# Test client application
#----------------------------------------------------------------------
add_executable(test_client
  "src/test_client.cc"
  ${dm_proto_srcs}
  ${dm_grpc_srcs}
  )
target_link_libraries(test_client
   ${_REFLECTION}
   ${_GRPC_GRPCPP}
   ${_PROTOBUF_LIBPROTOBUF})

#----------------------------------------------------------------------
# Test server application
#----------------------------------------------------------------------
add_executable(test_server
"src/test_server.cc"
${dm_proto_srcs}
${dm_grpc_srcs}
)
target_link_libraries(test_server
 ${_REFLECTION}
 ${_GRPC_GRPCPP}
 ${_PROTOBUF_LIBPROTOBUF})

add_dependencies(labview_grpc_server Detect_Compatibility_Breaks)
add_dependencies(labview_grpc_generator Detect_Compatibility_Breaks)
add_dependencies(test_client Detect_Compatibility_Breaks)
add_dependencies(test_server Detect_Compatibility_Breaks)
add_dependencies(example_client Detect_Compatibility_Breaks)