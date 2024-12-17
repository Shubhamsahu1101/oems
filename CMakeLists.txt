cmake_minimum_required(VERSION 3.10)

# Project name
project(OEMS)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add include directories (ensure paths are correct)
include_directories(
  /opt/homebrew/include
  /Users/shubhamsahu/Downloads/boost_1_82_0
)

# Link libraries
link_directories(/opt/homebrew/lib /opt/homebrew/Cellar/openssl@3/3.4.0/lib)

# Source files
set(SOURCES 
  src/main.cpp 
  src/OrderManager.cpp
  src/WebSocketClient.cpp
  src/WebSocketServer.cpp
  latency/Timing.cpp
)

# Add executable
add_executable(OEMS ${SOURCES})

# Link libraries to the executable
target_link_libraries(OEMS PRIVATE ssl crypto pthread)

