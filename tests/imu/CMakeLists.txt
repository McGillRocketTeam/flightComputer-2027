cmake_minimum_required(VERSION 3.20)
project(imu_tests C CXX)
enable_testing()
set(MODULES "${CMAKE_CURRENT_SOURCE_DIR}/../../modules")
add_executable(imu_test imu_test.cpp
    "${MODULES}/sensors/IMU.cpp"
    "${MODULES}/sensors/drivers/lsm6dsr/lsm6dsr_reg.c")
target_include_directories(imu_test PRIVATE "${MODULES}")
target_compile_features(imu_test PRIVATE cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(imu_test PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-Wall;-Wextra;-Werror>")
endif()
add_test(NAME imu_test COMMAND imu_test)
