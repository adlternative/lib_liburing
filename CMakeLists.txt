project(lib_lib_uring)
 cmake_minimum_required( VERSION 3.21 )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -march=native -O2 -pthread")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)
set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/include/)
#add_subdirectory(${PROJECT_SOURCE_DIR}/src)
#add_subdirectory(${PROJECT_SOURCE_DIR}/test)


link_libraries(uring)
link_libraries(fmt)
link_libraries(spdlog)
link_libraries(gtest)
link_libraries(gtest_main)

add_executable(uring_test ${PROJECT_SOURCE_DIR}/test/lib_lib_uring_test.cpp
													${PROJECT_SOURCE_DIR}/src/Cqe.cpp
													${PROJECT_SOURCE_DIR}/src/Sqe.cpp
													${PROJECT_SOURCE_DIR}/src/lib_lib_uring.cpp
							)