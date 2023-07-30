cmake_minimum_required(VERSION 3.22)
project(chat)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/vcpkg.json)
set(VCPKG_PREFER_SYSTEM_LIBS "CMAKE_LIBRARY_PATH") 

find_package(GameNetworkingSockets CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_subdirectory(common)

add_executable(
    server
    server/main.cpp server/server.cpp
)
target_link_libraries(server 
    PRIVATE GameNetworkingSockets::shared 
    PRIVATE CommonLibrary
)
target_include_directories(server PRIVATE 
    "${PROJECT_BINARY_DIR}" 
    "${PROJECT_BINARY_DIR}/common"
)

add_executable(
    client
    client/main.cpp client/client.cpp client/app.cpp
)
target_link_libraries(client 
    PRIVATE GameNetworkingSockets::shared 
    PRIVATE imgui::imgui
    PRIVATE glfw
    PRIVATE OpenGL::GL
    PRIVATE CommonLibrary
)
target_include_directories(client PRIVATE 
    "${PROJECT_BINARY_DIR}" 
    "${PROJECT_BINARY_DIR}/common"
)
