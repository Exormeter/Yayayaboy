cmake_minimum_required(VERSION 3.17.2)

project(GBEmu LANGUAGES CXX)

add_executable(GBEmu main.cpp)

set_property(TARGET GBEmu PROPERTY CXX_STANDARD 17)

# set(RELEASE_FLAGS "-Ofast -DNDEBUG -DBOOST_DISABLE_ASSERTS")
# set(DEBUG_FLAGS "-O0 -ggdb3")

# set(CMAKE_CXX_FLAGS_RELEASE ${RELEASE_FLAGS})
# set(CMAKE_C_FLAGS_RELEASE ${RELEASE_FLAGS})

# set(CMAKE_CXX_FLAGS_DEBUG ${DEBUG_FLAGS})
# set(CMAKE_C_FLAGS_DEBUG ${DEBUG_FLAGS})

# set(CMAKE_VERBOSE_MAKEFILE ON)

# add_compile_options(
#        -Wall -Werror -Wno-error=maybe-uninitialized
#        $<$<CONFIG:RELEASE>:-Ofast>
#        $<$<CONFIG:DEBUG>-O0>
#        $<$<CONFIG:DEBUG>-ggdb3>
# )
# add_compile_definitions(
#         $<$<CONFIG:RELEASE>:NDEBUG>
#         $<$<CONFIG:RELEASE>:BOOST_DISABLE_ASSERTS>
# )

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

target_link_libraries(GBEmu ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )


target_include_directories(GBEmu PUBLIC "./include")
target_include_directories(GBEmu PUBLIC "./include/MemoryController")
target_include_directories(GBEmu PUBLIC "./include/Peripheral")

add_subdirectory("src")

