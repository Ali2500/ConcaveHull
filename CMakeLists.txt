cmake_minimum_required(VERSION 2.8)
project(ConcaveHull)

if (CMAKE_VERSION VERSION_LESS "3.1")
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set (CMAKE_CXX_FLAGS "-std=gnu++11 ${CMAKE_CXX_FLAGS}")
  endif ()
else ()
  set (CMAKE_CXX_STANDARD 11)
endif ()

find_package(OpenCV REQUIRED)
list(APPEND INCLUDE_DIRS ${OpenCV_INCLUDE_DIRS})
list(APPEND LIBRARY_DIRS ${OpenCV_LIBRARY_DIRS})
list(APPEND LIBRARIES ${OpenCV_LIBRARIES})

find_package(Boost REQUIRED COMPONENTS program_options filesystem)
list(APPEND INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
list(APPEND LIBRARY_DIRS ${Boost_LIBRARY_DIRS})
list(APPEND LIBRARIES ${Boost_LIBRARIES})

add_Library(ConcaveHull_lib concave_hull_builder.cpp)
target_link_libraries(ConcaveHull_lib ${LIBRARIES})
list(APPEND LIBRARIES ${ConcaveHull_lib})

add_executable(${PROJECT_NAME} "main.cpp" "concave_hull_builder.cpp")
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})
