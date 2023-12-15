cmake_minimum_required(VERSION 2.6)

# Name of project
set(PROJ_NAME "The_Woodland_Revenant")
project(${PROJ_NAME})

# Specify project files: header files and source files
set(HDRS
    asteroid.h camera.h game.h resource.h resource_manager.h scene_graph.h scene_node.h model_loader.h 
    ghost.h interactable_node.h skybox.h entities.h renderable.h instanced_object.h)
 
set(SRCS
    asteroid.cpp camera.cpp game.cpp main.cpp resource.cpp resource_manager.cpp scene_graph.cpp scene_node.cpp 
    ghost.cpp interactable_node.cpp skybox.cpp entities.cpp renderable.cpp instanced_object.cpp)

# Add path name to configuration file
#configure_file(path_config.h.in path_config.h)

# Add executable based on the header and source files
add_executable(${PROJ_NAME} ${HDRS} ${SRCS})

# Require OpenGL library
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
target_link_libraries(${PROJ_NAME} ${OPENGL_gl_LIBRARY})

# Source Directory
add_compile_definitions(CMAKE_PROJ_DIRECTORY="${CMAKE_SOURCE_DIR}/..")
add_compile_definitions(CMAKE_SOURCE_DIRECTORY="${CMAKE_SOURCE_DIR}")

# Other libraries needed
set(LIBRARY_PATH C:\Users\nicho\Libraries)
#set(LIBRARY_PATH C:/Users/hikel/Desktop/University/Fall\ 2023/COMP\ 3501A/Libraries)
#set(LIBRARY_PATH C:/Users/kevin/OneDrive/Carleton/YEAR3/FALL/COMP3501/Libraries)
#set(LIBRARY_PATH C:\Libraries)
#set(LIBRARY_PATH C:/Program\ Files/Libraries/Libraries)
include_directories(${LIBRARY_PATH}/include)
if(NOT WIN32)
    find_library(GLEW_LIBRARY GLEW)
    find_library(GLFW_LIBRARY glfw)
    find_library(SOIL_LIBRARY SOIL)
    find_library(BASS_LIBRARY bass)
elseif(WIN32)
    find_library(GLEW_LIBRARY glew32s HINTS ${LIBRARY_PATH}/lib)
    find_library(GLFW_LIBRARY glfw3 HINTS ${LIBRARY_PATH}/lib)
    find_library(SOIL_LIBRARY SOIL HINTS ${LIBRARY_PATH}/lib)
    find_library(BASS_LIBRARY bass HINTS ${LIBRARY_PATH}/lib)
    
endif(NOT WIN32)
target_link_libraries(${PROJ_NAME} ${GLEW_LIBRARY})
target_link_libraries(${PROJ_NAME} ${GLFW_LIBRARY})
target_link_libraries(${PROJ_NAME} ${SOIL_LIBRARY})
target_link_libraries(${PROJ_NAME} ${BASS_LIBRARY})

# The rules here are specific to Windows Systems
if(WIN32)
    # Avoid ZERO_CHECK target in Visual Studio
    set(CMAKE_SUPPRESS_REGENERATION TRUE)
 
    # This will use the proper libraries in debug mode in Visual Studio
    set_target_properties(${PROJ_NAME} PROPERTIES DEBUG_POSTFIX _d)
endif(WIN32)

if(EXISTS "${LIBRARY_PATH}/lib/bass.lib" AND EXISTS "${LIBRARY_PATH}/include/BASS/bass.h")
    add_compile_definitions(USE_SOUND)
endif()

configure_file(${CMAKE_SOURCE_DIR}/dlls/bass.dll ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)