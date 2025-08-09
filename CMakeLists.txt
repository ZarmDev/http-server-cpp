cmake_minimum_required(VERSION 3.13)

project(http-server-starter-cpp)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard

# Include headers for redis-plus-plus
include_directories("/home/d/Applications/redis-plus-plus/include")
include_directories("/usr/include/hiredis")

# Link libraries for redis-plus-plus
link_directories(/home/d/Applications/redis-plus-plus/lib)
link_directories(/usr/include/hiredis/lib)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

add_executable(server ${SOURCE_FILES})

target_link_libraries(server PRIVATE Threads::Threads ZLIB::ZLIB)
target_link_libraries(server PRIVATE hiredis redis++ pthread)