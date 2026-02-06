cmake_minimum_required(VERSION 3.8)
project(mbot_hardware)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 1. 找 ROS 依赖
find_package(ament_cmake REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)

# ================= 关键修改：用万能钥匙找 libserial =================
find_package(PkgConfig REQUIRED)
# 这句话的意思是：去系统里搜 "libserial" 这个名字，把它的路径全存到 LIB_SERIAL 变量里
pkg_check_modules(LIB_SERIAL REQUIRED libserial)
# ================================================================

# 2. 编译库
add_library(${PROJECT_NAME} SHARED
  src/mbot_system.cpp
)

# 3. 指定头文件路径
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  # 把刚才万能钥匙找到的头文件路径加进来
  ${LIB_SERIAL_INCLUDE_DIRS}
)

# 4. 链接库文件
target_link_libraries(${PROJECT_NAME} PRIVATE
  rclcpp::rclcpp
  pluginlib::pluginlib
  hardware_interface::hardware_interface
  rclcpp_lifecycle::rclcpp_lifecycle
  # 把刚才万能钥匙找到的库文件链接进来
  ${LIB_SERIAL_LIBRARIES}
)

# 5. 注册插件
pluginlib_export_plugin_description_file(hardware_interface mbot_hardware.xml)

# 6. 安装规则
install(
  TARGETS ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  FILES mbot_hardware.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(rclcpp hardware_interface pluginlib)

ament_package()