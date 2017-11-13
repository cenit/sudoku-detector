cmake_minimum_required(VERSION 3.1)
project(SudokuSolver)
set(CMAKE_CXX_STANDARD 11)

macro(configure_link_flags)
# Set compiler options.
set(MSVC_C_CXX_FLAGS
  CMAKE_C_FLAGS_DEBUG
  CMAKE_C_FLAGS_MINSIZEREL
  CMAKE_C_FLAGS_RELEASE
  CMAKE_C_FLAGS_RELWITHDEBINFO
  CMAKE_CXX_FLAGS_DEBUG
  CMAKE_CXX_FLAGS_MINSIZEREL
  CMAKE_CXX_FLAGS_RELEASE
  CMAKE_CXX_FLAGS_RELWITHDEBINFO
)
if(${VCPKG_TARGET_TRIPLET} MATCHES "static")
  message(STATUS
    "VCPKG: static link"
  )
  foreach(flag ${MSVC_C_CXX_FLAGS})
    if(${flag} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
    endif()
  endforeach()
else()
  message(STATUS
    "VCPKG: dynamic link"
  )
  foreach(flag ${MSVC_C_CXX_FLAGS})
    if(${flag} MATCHES "/MT")
      string(REGEX REPLACE "/MT" "/MD" ${flag} "${${flag}}")
    endif()
  endforeach()
endif()
endmacro()

macro(print_link_flags)
set(MSVC_C_CXX_FLAGS
  CMAKE_C_FLAGS_DEBUG
  CMAKE_C_FLAGS_MINSIZEREL
  CMAKE_C_FLAGS_RELEASE
  CMAKE_C_FLAGS_RELWITHDEBINFO
  CMAKE_CXX_FLAGS_DEBUG
  CMAKE_CXX_FLAGS_MINSIZEREL
  CMAKE_CXX_FLAGS_RELEASE
  CMAKE_CXX_FLAGS_RELWITHDEBINFO
)
message(STATUS "Initial build flags:")
foreach(flag ${MSVC_C_CXX_FLAGS})
  message(STATUS "  '${flag}': ${${flag}}")
endforeach()
message(STATUS "")
endmacro()

if(MSVC)
add_definitions(-D_SCL_SECURE_NO_WARNINGS)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
configure_link_flags()
#print_link_flags()
endif()

find_package(OpenCV 3 REQUIRED HINTS /usr/local/opt/opencv3)
#find_package(OpenCV REQUIRED HINTS /usr/local/opt/opencv)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(sudokusolver
  ${CMAKE_SOURCE_DIR}/src/sudokusolver.cpp
  ${CMAKE_SOURCE_DIR}/src/sudoku-tools.cpp
)

target_link_libraries(sudokusolver
  ${OpenCV_LIBS}
)
