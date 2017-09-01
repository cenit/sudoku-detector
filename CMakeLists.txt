cmake_minimum_required(VERSION 2.8)
project(SudokuSolver)
set(CMAKE_CXX_STANDARD 11)

if(MSVC)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

find_package(OpenCV 3 REQUIRED HINTS /usr/local/opt/opencv3)
#find_package(OpenCV REQUIRED HINTS /usr/local/opt/opencv)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(sudokusolver
  ${CMAKE_SOURCE_DIR}/src/sudokusolver.cpp
)

target_link_libraries(sudokusolver 
  ${OpenCV_LIBS}
)

