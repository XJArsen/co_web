# add_executable(server ./server.cpp)
# add_executable(client ./client.cpp)
# add_executable(ThreadPoolTest ./ThreadPoolTest.cpp)
add_executable(main ./main.cpp)

# target_link_libraries(server share_src)
# target_link_libraries(client share_src)
# target_link_libraries(ThreadPoolTest share_src)
target_link_libraries(main share_src)
target_link_libraries(main  libmysqlclient.so)



set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
