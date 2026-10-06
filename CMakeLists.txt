# 1. Khai báo phiên bản tối thiểu
cmake_minimum_required(VERSION 3.16)

project(sysst8 C)

# ANCHOR - Bật xuất compile_commands.json cho VS Code IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

# ANCHOR - Thu thập file nguồn chung
file(GLOB_RECURSE SRCS 
  "src/*.c"
)

add_library(${PROJECT_NAME} STATIC ${SRCS})

# ANCHOR - Cấu hình Include Directories chung & riêng
target_include_directories(${PROJECT_NAME} PUBLIC
  "config/"
  "inc/"
)
