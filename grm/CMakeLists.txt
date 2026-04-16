cmake_minimum_required(VERSION 3.14)
project(grm CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TENSOR_SIM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../tensor_sim")

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -O2)
endif()

# Tensor Core simulator libraries (compiled from source)
add_library(volta_sim   STATIC "${TENSOR_SIM_DIR}/src/volta_tensor.cpp")
target_include_directories(volta_sim PUBLIC "${TENSOR_SIM_DIR}/include")

add_library(ampere_sim  STATIC "${TENSOR_SIM_DIR}/src/ampere_tensor.cpp")
target_include_directories(ampere_sim PUBLIC "${TENSOR_SIM_DIR}/include")

add_library(hopper_sim  STATIC "${TENSOR_SIM_DIR}/src/hopper_tensor.cpp")
target_include_directories(hopper_sim PUBLIC "${TENSOR_SIM_DIR}/include")

add_library(blackwell_sim STATIC "${TENSOR_SIM_DIR}/src/blackwell_tensor.cpp")
target_include_directories(blackwell_sim PUBLIC "${TENSOR_SIM_DIR}/include")

# GRM library
add_library(grm_lib STATIC
    src/probes_fp16.cpp
    src/probes_bf16.cpp
    src/fingerprint.cpp
)
target_include_directories(grm_lib PUBLIC
    include
    "${TENSOR_SIM_DIR}/include"
)
target_link_libraries(grm_lib PUBLIC
    volta_sim ampere_sim hopper_sim blackwell_sim
)

# GRM executable
add_executable(grm main.cpp)
target_link_libraries(grm PRIVATE grm_lib)
target_include_directories(grm PRIVATE include)

if(MINGW)
    target_link_options(grm PRIVATE -static-libgcc -static-libstdc++)
endif()
