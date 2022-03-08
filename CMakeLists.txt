cmake_minimum_required(VERSION 3.10)
set(TARGET tetravex-solver)
project(${TARGET})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)

set(SRC
  src/main.cc
  src/tetravex.cc
  )

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set_source_files_properties(${SRC}
    PROPERTIES COMPILE_FLAGS "-Wall -Wextra -pedantic -Werror -g"
    )
endif ()

add_executable(${TARGET}
  ${SRC}
  )

target_include_directories(${TARGET} PUBLIC
  "${PROJECT_SOURCE_DIR}/src/include"
  )

# RUN DEBUG TARGET
add_custom_target(run
  COMMAND printf %37sOUTPUT%37s\\\\n | tr " " "="
  COMMAND ./bin/${TARGET}
  COMMAND printf %80s\\\\n | tr " " "="
  DEPENDS ${TARGET}
  )

# RUN DEBUG TARGET
add_custom_target(run-debug
  COMMAND printf %36sOUTPUT%37s\\\\n | tr " " "="
  COMMAND valgrind --leak-check=full --show-leak-kinds=all --log-file=leaks.log  --suppressions=etc/valgrind.supp ./bin/${TARGET}
  COMMAND printf %38sLEAK%38s\\\\n | tr " " "="
  COMMAND ./etc/script/stat-leaks.sh
  COMMAND printf %80s\\\\n | tr " " "="
  DEPENDS ${TARGET}
  )
