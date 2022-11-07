cmake_minimum_required(VERSION 3.20)
project(crs LANGUAGES CXX C DESCRIPTION "custom reverse shell")

add_executable(crs_server server/main.cxx server/server.cxx server/handler.cxx)
add_executable(crs_client client/main.c)

target_compile_features(crs_server PRIVATE cxx_std_17)
target_compile_definitions(crs_server PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN)
target_link_libraries(crs_server Ws2_32)

target_compile_features(crs_client PRIVATE c_std_99)
target_compile_definitions(crs_client PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN)
target_link_libraries(crs_client Ws2_32)