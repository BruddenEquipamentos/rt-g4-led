cmake_minimum_required(VERSION 3.10)
project(desktop VERSION 1.0)

# include_directories(../shared/nvm/ST/EEPROM_Emul/Core)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB source_files
          "../shared/ble/ble_protocol/*.h"
          "../shared/ble/ble_protocol/*.c"
          "../shared/ble/*.h"
          "../shared/ble/*.c"
          "../shared/modbus/*.h"
          "../shared/modbus/*.c"
          "../shared/nvm/*.h"
          "../shared/nvm/*.c"
          "../shared/panel/*.h"
          "../shared/panel/*.c"
          "../shared/sd/*.h"
          "../shared/sd/*.c"
          "../shared/dm/*.h"
          "../shared/dm/*.c"
          "../shared/scene/*.h"
          "../shared/scene/*.c"
          "../shared/*.h"
          "../shared/*.c"
          "*.h"
          "*.c")

add_compile_options(-g -Wall -Wextra -pedantic -Werror -Wno-unused-parameter -Wno-unused-function -Wno-gnu-binary-literal)

add_executable(main ${source_files})

target_include_directories(main PRIVATE
          "${CMAKE_CURRENT_SOURCE_DIR}"
          "${CMAKE_CURRENT_SOURCE_DIR}/../shared"
          "${CMAKE_CURRENT_SOURCE_DIR}/../shared/ble"
          "${CMAKE_CURRENT_SOURCE_DIR}/../shared/ble/ble_protocol")

target_compile_options(main PRIVATE -include "${CMAKE_CURRENT_SOURCE_DIR}/compat.h")

if (CMAKE_C_COMPILER_ID MATCHES "Clang")
          target_compile_options(main PRIVATE -Wno-c23-extensions)
endif()
