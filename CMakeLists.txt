cmake_minimum_required(VERSION 3.22)

project(PersonalHearingAssistantRoot VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add Desktop C++ Application
add_subdirectory(desktop)

# Enable Tests
enable_testing()
add_subdirectory(tests)
