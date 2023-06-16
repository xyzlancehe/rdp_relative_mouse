cmake_minimum_required(VERSION 3.10)
project(MouseTest)

set(CMAKE_CXX_STANDARD 17)

link_directories(interception)

add_executable(main "main.cpp")
add_executable(calibrate "calibrate.cpp")
add_executable(mouse_info "mouse_info.cpp")

target_link_libraries(main interception)
target_link_libraries(calibrate interception)
target_link_libraries(mouse_info interception)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)