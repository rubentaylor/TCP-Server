cmake_minimum_required(VERSION 3.16)

project(TCPChatServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

add_executable(server
    src/main.cpp
    src/TCPListener.cpp
    src/ClientChat.cpp
)

target_include_directories(server PRIVATE include)
target_link_libraries(server PRIVATE Threads::Threads)

if(WIN32)
    target_link_libraries(server PRIVATE ws2_32)
endif()
