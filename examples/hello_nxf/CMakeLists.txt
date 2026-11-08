add_executable(hello_nxf main.cpp)
target_link_libraries(hello_nxf PRIVATE nxf::core)
nxf_set_warnings(hello_nxf)
