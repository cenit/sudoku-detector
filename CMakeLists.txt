cmake_minimum_required(VERSION 3.1)
project(SudokuSolver)
set(CMAKE_CXX_STANDARD 11)


if (EXISTS "$ENV{WORKSPACE}/sysconfig/cmake/physycom_config.cmake")
   message(STATUS "Enabling physycom settings")
   include("$ENV{WORKSPACE}/sysconfig/cmake/physycom_config.cmake")
else()
   message(STATUS "Unable to find physycom settings file")
endif()

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(sudokusolver
  ${CMAKE_SOURCE_DIR}/src/sudokusolver.cpp
  ${CMAKE_SOURCE_DIR}/src/sudoku-tools.cpp
)

target_link_libraries(sudokusolver
  ${OpenCV_LIBS}
)

install(TARGETS sudokusolver DESTINATION ${CMAKE_SOURCE_DIR}/bin/)
