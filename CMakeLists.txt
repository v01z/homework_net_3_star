cmake_minimum_required(VERSION 3.22)
project(homework_net_3_star)

set(CMAKE_CXX_STANDARD 17)

add_executable(server main_server.cpp TCP_Server.cpp err_decoding.cpp)
add_executable(client main_client.cpp err_decoding.cpp)

#We will use mingw on Win
if(MINGW)
    target_link_libraries(server ws2_32)
    target_link_libraries(client ws2_32)
endif()
