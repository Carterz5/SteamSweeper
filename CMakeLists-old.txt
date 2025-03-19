cmake_minimum_required(VERSION 3.10)

# Project name
project(dps_calc C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Source files
set(SOURCES src/main.c)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Detect platform and apply platform-specific settings
if(WIN32)
    message(STATUS "Compiling for Windows")    
    # Remove default shared library prefix
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    
    # Mark the executable as a Windows GUI application
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE ON)
    
    # Library to link against for Windows
    set(CIMGUI_LIB ${CMAKE_SOURCE_DIR}/include/cimgui_sdl.dll)
else()
    message(STATUS "Compiling for Linux")
    
    # Library to link against for Linux
    set(CIMGUI_LIB ${CMAKE_SOURCE_DIR}/include/libcimgui_sdl.so)
endif()

# Find required libraries
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# Remove SDL2main if it's included
list(REMOVE_ITEM SDL2_LIBRARIES SDL2::SDL2main)

# Compilation definitions
target_compile_definitions(${PROJECT_NAME} PUBLIC CIMGUI_USE_OPENGL3 CIMGUI_USE_SDL2)

# Link against libraries
target_link_libraries(${PROJECT_NAME} 
    ${SDL2_LIBRARIES} 
    ${OPENGL_LIBRARIES} 
    ${GLEW_LIBRARIES} 
    ${CIMGUI_LIB})

# Set rpath to find shared libraries at runtime
set_target_properties(${PROJECT_NAME} PROPERTIES 
    BUILD_RPATH "${CMAKE_SOURCE_DIR}/include"
)

# Custom clean target
add_custom_target(clean-all 
    COMMAND ${CMAKE_COMMAND} -E remove ${PROJECT_NAME} ${CMAKE_BINARY_DIR}/*.o
    COMMENT "Cleaning up binaries and object files"
)