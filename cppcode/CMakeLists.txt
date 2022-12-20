cmake_minimum_required(VERSION 3.3)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
project(cppex)
find_package(SCIP REQUIRED)
include_directories(${SCIP_INCLUDE_DIRS})

add_library(solutions solutions.cpp)
add_library(v_history var_history.cpp)
add_library(event_solfeedback event_solfeedback.cpp)

add_executable(cppex
  main.cpp)
target_link_libraries(cppex ${SCIP_LIBRARIES})
target_link_libraries(cppex solutions)
target_link_libraries(cppex v_history)
target_link_libraries(cppex event_solfeedback)
if( TARGET examples )
    add_dependencies( examples cppex )
endif()
