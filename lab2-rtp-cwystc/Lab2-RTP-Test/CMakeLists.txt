cmake_minimum_required(VERSION 3.11)
project(assignment2-rtp)

enable_testing()
add_subdirectory(third_party/googletest-release-1.12.1)
include_directories(third_party/googletest-release-1.12.1/googletest/include)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
find_package(Threads REQUIRED)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)


#添加头文件搜索路径
include_directories(/usr/local/include)

#添加库文件搜索路径
link_directories(/usr/local/lib)
include(GoogleTest)
add_library(rtpall
#		src/rtp.c
#		src/rtp_more_file_1.c
#		src/rtp_more_file_2.c
		src/util.c
)

add_library(rtpsender src/sender_def.c)
target_link_libraries(rtpsender PUBLIC rtpall)

add_library(rtpreceiver src/receiver_def.c)
target_link_libraries(rtpreceiver PUBLIC rtpall)

add_executable(rtp_test_all
		src/test.cpp
)
target_link_libraries(rtp_test_all PUBLIC rtpsender rtpreceiver)
target_link_libraries(rtp_test_all PUBLIC Threads::Threads GTest::gtest_main)

gtest_discover_tests(rtp_test_all)
