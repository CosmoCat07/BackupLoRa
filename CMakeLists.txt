# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(BackupLoRa C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()
add_compile_options(
    -Wall
    -Wno-format          
    -Wno-unused-function
)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/RadioLib")
# Add executable. Default name is the project name, version 0.1
add_executable(BackupLoRa BackupLoRa.cpp 
"${CMAKE_CURRENT_SOURCE_DIR}/RadioLib/src/hal/RPiPico/PicoHal.cpp")

target_compile_definitions(${PROJECT_NAME} PUBLIC RADIOLIB_BUILD_RPI_PICO)



pico_set_program_name(BackupLoRa "BackupLoRa")
pico_set_program_version(BackupLoRa "0.1")

# Modify the belohardware_spiw lines to enable/disable output over UART/USB
pico_enable_stdio_uart(BackupLoRa 0)
pico_enable_stdio_usb(BackupLoRa 1)


# Add the standard include files to the build
target_include_directories(BackupLoRa PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${PROJECT_NAME} pico_stdlib hardware_spi hardware_gpio hardware_timer pico_multicore hardware_pwm RadioLib)

pico_add_extra_outputs(BackupLoRa)

