cmake_minimum_required(VERSION 3.11)
SET(PSL_ASM "assembly")
set(LOCAL_PROJECT ${PSL_ASM})
project(${LOCAL_PROJECT} VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)


FetchContent_Declare(googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
		GIT_TAG master) # need master for benchmark::benchmark
FetchContent_MakeAvailable(
			googlebenchmark)

add_executable(${LOCAL_PROJECT} main.cpp)

target_link_libraries(${LOCAL_PROJECT} ${PSL_PROJECT} benchmark::benchmark)