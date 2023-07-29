cmake_minimum_required(VERSION 3.22)
project(chat)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

# set(VCPKG_MANIFEST_MODE "OFF")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/vcpkg.json)
set(VCPKG_PREFER_SYSTEM_LIBS "CMAKE_LIBRARY_PATH") 

# set(CMAKE_TOOLCHAIN_FILE "../vcpkg/scripts/buildsystems/vcpkg.cmake")
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

find_package(GameNetworkingSockets CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(
    server
    server/main.cpp server/server.cpp
)

add_executable(
    client
    client/main.cpp client/client.cpp client/app.cpp
)

target_link_libraries(server 
    PRIVATE GameNetworkingSockets::shared 
)

target_link_libraries(client 
    PRIVATE GameNetworkingSockets::shared 
    PRIVATE imgui::imgui
    PRIVATE glfw
    PRIVATE OpenGL::GL
)
