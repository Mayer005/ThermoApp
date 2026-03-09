# Define minimal required version of CMake.
cmake_minimum_required(VERSION "3.25")

# Project definition
project(
	ThermoApp
	VERSION 1.0
	LANGUAGES C CXX ASM
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE APP_CPP_SOURCES "../src/*.cpp")
file(GLOB_RECURSE APP_C_SOURCES "../src/*.c")

# Include the definition of the slc_ThermoApp target,
# which contains the content of the SLC project
include(ThermoApp.cmake)

add_executable(ThermoApp
    ${APP_CPP_SOURCES}
    ${APP_C_SOURCES}
)

target_include_directories(ThermoApp PUBLIC
    ../src
)

target_compile_definitions(ThermoApp PUBLIC
    # Add additional macros here
)

target_compile_options(ThermoApp PUBLIC
    # Set additional compiler flags here
)

target_link_options(ThermoApp PUBLIC
    # Set additional linker flags here
)

# Link with the content defined in the SLC project
target_link_libraries(ThermoApp PRIVATE
    slc
)

# Include managed project content if available
include(ThermoApp_project.cmake OPTIONAL RESULT_VARIABLE managed_project)
if(managed_project)
    message(STATUS "Using managed project content from ${managed_project}")
endif()

# Force the gcc linker command
set_target_properties(ThermoApp PROPERTIES LINKER_LANGUAGE CXX)

# Create .bin, .hex and .s37 artifacts after building the project
add_custom_command(TARGET ThermoApp
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} ${OBJCOPY_SREC_CMD} "$<TARGET_FILE:ThermoApp>" "$<TARGET_FILE_DIR:ThermoApp>/$<TARGET_FILE_BASE_NAME:ThermoApp>.s37"
    COMMAND ${CMAKE_OBJCOPY} ${OBJCOPY_IHEX_CMD} "$<TARGET_FILE:ThermoApp>" "$<TARGET_FILE_DIR:ThermoApp>/$<TARGET_FILE_BASE_NAME:ThermoApp>.hex"
    COMMAND ${CMAKE_OBJCOPY} ${OBJCOPY_BIN_CMD}  "$<TARGET_FILE:ThermoApp>" "$<TARGET_FILE_DIR:ThermoApp>/$<TARGET_FILE_BASE_NAME:ThermoApp>.bin" 
)

# Run post-build pipeline to perform additional post-processing
if(post_build_command)
add_custom_command(TARGET ThermoApp
    POST_BUILD
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/..
    COMMAND ${post_build_command}
)
endif()