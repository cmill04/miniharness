cmake_minimum_required(VERSION 3.16)
project(miniharness CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-fsanitize=address -fsanitize=undefined -g)
add_link_options(-fsanitize=address -fsanitize=undefined)

add_library(miniharness_core
    src/sentinel_scanner.cpp
    src/model_client.cpp
    src/scripted_client.cpp
    src/replay_client.cpp
    src/harness.cpp
    src/conversation.cpp
)
target_include_directories(miniharness_core PUBLIC include)

add_executable(miniharness src/main.cpp)
target_link_libraries(miniharness PRIVATE miniharness_core)

add_executable(test_p1 tests/p1/test_p1.cpp)
target_link_libraries(test_p1 PRIVATE miniharness_core)

