cmake_minimum_required(VERSION 3.10)
project(spdz_codebase)

# set cmake flags
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp -lboost_program_options -fsanitize=address -g -fno-common -fno-omit-frame-pointer")

# set source files and spdz home (need to revise SPDZ_HOME to your local path)
set(SOURCE_FILES spdz_connector.h spdz_connector.cc common.h)
set(SPDZ_HOME "/home/wuyuncheng/Documents/projects/MP-SPDZ/")

# add library connector
add_library(connector ${SOURCE_FILES})

# add include directories to the target executor
target_include_directories(connector PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${SPDZ_HOME}
        ${SPDZ_HOME}/local/include)

# link required libraries for mp-spdz connector
target_link_libraries(connector
        ${SPDZ_HOME}/libSPDZ.a
        ssl
        mpirxx
        mpir
        sodium)

# add directories in which the linker will look for libraries
link_directories(/usr/local/lib
        /usr/ssl/lib
        ${SPDZ_HOME}
        ${SPDZ_HOME}/local/lib)

# add include directories to the build
include_directories(
        ${PROJECT_SOURCE_DIR}
        ${SPDZ_HOME}
        ${SPDZ_HOME}/local/include)

# add executable and link to connector
add_executable(spdz_codebase main.cpp)
target_link_libraries(spdz_codebase connector)
