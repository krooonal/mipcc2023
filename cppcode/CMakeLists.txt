cmake_minimum_required(VERSION 3.3)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
project(cppex)
find_package(SCIP REQUIRED)
include_directories(${SCIP_INCLUDE_DIRS})

add_library(solutions solutions.cpp)
add_library(v_history var_history.cpp)
# add_library(cuts_pool cuts_pool.cpp)
# add_library(event_solfeedback event_solfeedback.cpp)
# add_library(branch_cumpscost branch_cumpscost.cpp)
# add_library(branch_levelpscost branch_levelpscost.cpp)
add_library(parameters parameters.cpp)

add_executable(cppex
  main.cpp)
target_link_libraries(cppex ${SCIP_LIBRARIES})
target_link_libraries(cppex solutions)
target_link_libraries(cppex v_history)
# target_link_libraries(cppex cuts_pool)
# target_link_libraries(cppex event_solfeedback)
target_link_libraries(cppex parameters)
# target_link_libraries(cppex branch_cumpscost)
# target_link_libraries(cppex branch_levelpscost)

add_executable(experiment experiment.cpp)
target_link_libraries(experiment ${SCIP_LIBRARIES})

add_executable(basecppex basemain.cpp)
target_link_libraries(basecppex ${SCIP_LIBRARIES})

if( TARGET examples )
    add_dependencies( examples cppex )
endif()
