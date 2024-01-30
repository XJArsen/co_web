cmake_minimum_required(VERSION 3.22.1)
file(GLOB_RECURSE ALL_SOURCE CONFIGURE_DEPENDS  "*.cpp" "*.c")
add_library(share_src ${ALL_SOURCE}) 

