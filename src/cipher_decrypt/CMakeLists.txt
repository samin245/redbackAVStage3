cmake_minimum_required(VERSION 3.8)
project(cipher_decrypt)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cipher_interfaces REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(cipher src/decrypt.cpp)
ament_target_dependencies(cipher rclcpp std_msgs cipher_interfaces)


install(TARGETS
  cipher
  DESTINATION lib/${PROJECT_NAME})

ament_package()