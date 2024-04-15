cmake_minimum_required(VERSION 3.22)
set(CMAKE_C_COMPILER "/usr/bin/clang-18")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++-18")
SET(CMAKE_TOOLCHAIN_FILE "/home/fanghaizhou/software/vcpkg/scripts/buildsystems/vcpkg.cmake")
project(EMDS)
set(CMAKE_CXX_STANDARD 20)
SET(CMAKE_BUILD_TYPE "Debug")
# SET(CMAKE_BUILD_TYPE "Release")
include_directories(SYSTEM "/home/fanghaizhou/software/vcpkg/installed/x64-linux/include")
find_package(Threads REQUIRED)
find_package(absl CONFIG REQUIRED)
find_package(Arrow CONFIG REQUIRED)
find_package(Parquet CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(cppzmq CONFIG REQUIRED)
find_package(folly CONFIG REQUIRED)
find_package(xsimd CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(TBB CONFIG REQUIRED)
find_package(websocketpp CONFIG REQUIRED)
find_package(simdjson CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(glog CONFIG REQUIRED)
set(GFLAGS_USE_TARGET_NAMESPACE ON)
find_package(gflags CONFIG REQUIRED)


set(absl_LIBRARIES
absl::algorithm
absl::base
absl::debugging
absl::flat_hash_map
absl::memory
absl::meta
absl::numeric
absl::str_format
absl::strings
absl::synchronization
absl::time
absl::flags
absl::strings
absl::log
absl::flags_internal
absl::log_initialize
absl::config
absl::base
absl::flags_parse
absl::utility)

include_directories(${CMAKE_SOURCE_DIR}/common/parallel_hashmap)
include_directories(${CMAKE_SOURCE_DIR}/common)
add_executable(test main.cc 
    common/utils.cc
    common/time_utils.cc
    common/parquet_io.cc
    common/df_alg.cc
    symbols_process.cc
)
target_compile_definitions(test PRIVATE -DGLOG_NO_EXPORT)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(test PRIVATE -O3 -march=native)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(test PRIVATE -O3 -march=native)
endif ()
target_link_libraries(test ${absl_LIBRARIES})
target_link_libraries(test fmt::fmt)
target_link_libraries(test "$<IF:$<BOOL:${ARROW_BUILD_STATIC}>,Arrow::arrow_static,Arrow::arrow_shared>")
target_link_libraries(test "$<IF:$<BOOL:${ARROW_BUILD_STATIC}>,Parquet::parquet_static,Parquet::parquet_shared>")
target_link_libraries(test Folly::folly Folly::folly_deps Folly::follybenchmark Folly::folly_test_util)
target_link_libraries(test xsimd)
target_link_libraries(test cppzmq cppzmq-static)
target_link_libraries(test nlohmann_json::nlohmann_json)
target_link_libraries(test TBB::tbb TBB::tbbmalloc)
target_link_libraries(test websocketpp::websocketpp)
target_link_libraries(test simdjson::simdjson)
target_link_libraries(test glog::glog)
target_link_libraries(test OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(test gflags::gflags)


