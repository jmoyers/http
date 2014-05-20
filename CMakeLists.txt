cmake_minimum_required(VERSION 2.6)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY build)

project (multiplexers)

project (kqueue)
file( GLOB KQUEUE_SRCS src/kqueue/*.c src/kqueue/*.h )
add_executable( kqueue ${KQUEUE_SRCS} )

project (uvs)

file( GLOB UVS_SRCS src/uv/*.c src/uv/*.h )

include_directories(${CMAKE_SOURCE_DIR}/build/vendor/libuv/include)

add_executable( uvs ${UVS_SRCS} )

include(ExternalProject)

ExternalProject_Add(libuv_vendor
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/vendor/libuv
  CONFIGURE_COMMAND ${CMAKE_SOURCE_DIR}/vendor/libuv/autogen.sh && ${CMAKE_SOURCE_DIR}/vendor/libuv/configure
  BUILD_COMMAND make
  INSTALL_COMMAND make install prefix=${CMAKE_SOURCE_DIR}/build/vendor/libuv
)

ExternalProject_Get_Property(libuv_vendor install_dir)

add_dependencies(uvs libuv_vendor)

target_link_libraries(uvs ${CMAKE_SOURCE_DIR}/build/vendor/libuv/lib/libuv.a)
