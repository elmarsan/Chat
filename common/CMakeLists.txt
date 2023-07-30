add_library(CommonLibrary STATIC message.cpp)

target_include_directories(CommonLibrary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(CommonLibrary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/nlohmann)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(message_test message_test.cpp message.cpp)
target_link_libraries(
    message_test
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(message_test)
