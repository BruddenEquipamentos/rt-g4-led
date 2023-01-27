cmake_minimum_required(VERSION 3.10)
project(desktop VERSION 1.0)

file(GLOB source_files
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

add_compile_options(-g -Wall -Wextra -pedantic -Werror -Wno-unused-parameter -Wno-gnu-binary-literal)

add_executable(main ${source_files})
