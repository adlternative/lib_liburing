project(lib_lib_uring)
 cmake_minimum_required( VERSION 3.21 )
#-fsanitize=leak
#-fsanitize=address
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -march=native -O2 -pthread -g ")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)
set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/include/)
#add_subdirectory(${PROJECT_SOURCE_DIR}/src)
#add_subdirectory(${PROJECT_SOURCE_DIR}/test)


add_executable(uring_test ${PROJECT_SOURCE_DIR}/test/lib_lib_uring_test.cpp
													${PROJECT_SOURCE_DIR}/src/AsyncIORequest.cpp
													${PROJECT_SOURCE_DIR}/src/lib_lib_uring.cpp
							)

target_link_libraries(
	uring_test
	uring
	fmt
	spdlog
	gtest
	gtest_main
)


add_executable(uring_test2 ${PROJECT_SOURCE_DIR}/test/lib_lib_uring_cp.cpp
													${PROJECT_SOURCE_DIR}/src/AsyncIORequest.cpp
													${PROJECT_SOURCE_DIR}/src/lib_lib_uring.cpp
							)

target_link_libraries(
	uring_test2
	uring
	fmt
	spdlog
	gtest
)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
