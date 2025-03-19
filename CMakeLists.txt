cmake_minimum_required(VERSION 3.10)

# Project name
project(SteamSweep C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include)

# Source files - get all .c files in src/
file(GLOB SOURCES src/*.c)

#set output dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})


#close if not win32
if(NOT WIN32)
    message(FATAL_ERROR "This is not Windows, please compile on Windows")
endif()


# Dynamically find the correct MSYS2 UCRT64 bin directory
find_program(UCRT_GCC_PATH NAMES gcc HINTS "C:/msys64/ucrt64/bin" REQUIRED)
get_filename_component(UCRT_BIN_DIR ${UCRT_GCC_PATH} DIRECTORY)

# Install UCRT64 runtime DLLs dynamically
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${UCRT_BIN_DIR}/libgcc_s_seh-1.dll"
        "${UCRT_BIN_DIR}/libstdc++-6.dll"
        "${UCRT_BIN_DIR}/libwinpthread-1.dll"
        "${UCRT_BIN_DIR}/SDL2.dll"
        "${CMAKE_SOURCE_DIR}/include/cimgui_sdl.dll"
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Check if DLLs exist in output
if(NOT EXISTS "${UCRT_BIN_DIR}/libgcc_s_seh-1.dll")
    message(FATAL_ERROR "Could not find MinGW UCRT64 runtime DLLs. Ensure MSYS2 UCRT64 is installed and properly configured.")
endif()

    
# Remove default shared library prefix
set(CMAKE_SHARED_LIBRARY_PREFIX "")

# Mark the executable as a Windows GUI application
set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE ON)

# Library to link against for Windows
set(CIMGUI_LIB ${CMAKE_SOURCE_DIR}/include/cimgui_sdl.dll)


# Function to check if a package exists and install it (MSYS2)
function(check_and_install_package PACKAGE_NAME PACMAN_NAME)
    find_package(${PACKAGE_NAME} QUIET)
    if(NOT ${PACKAGE_NAME}_FOUND)
        message(WARNING "${PACKAGE_NAME} not found! Attempting to install using pacman...")
        execute_process(COMMAND "pacman" "-S" "--noconfirm" "${PACMAN_NAME}"
                        RESULT_VARIABLE PACMAN_RESULT
                        OUTPUT_VARIABLE PACMAN_OUTPUT
                        ERROR_VARIABLE PACMAN_ERROR
                        OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(PACMAN_RESULT EQUAL 0)
            message(STATUS "${PACKAGE_NAME} installed successfully.")
        else()
            message(FATAL_ERROR "Failed to install ${PACKAGE_NAME}. Run 'pacman -S ${PACMAN_NAME}' manually.")
        endif()
    else()
        message(STATUS "${PACKAGE_NAME} is already installed.")
    endif()
endfunction()


#install packages if they don't exist
check_and_install_package(SDL2 mingw-w64-ucrt-x86_64-SDL2)
check_and_install_package(GLEW mingw-w64-ucrt-x86_64-glew)

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

message("cmake complete")