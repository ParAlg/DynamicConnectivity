add_subdirectory(seq_hdt)
# add_executable(bench_static_connectivity bench_static_connectivity.cc)
# target_link_libraries(bench_static_connectivity PRIVATE dycon )

add_executable(bench_rwg bench_rwg.cc)
target_link_libraries(bench_rwg PRIVATE dycon lib_dynamic_connectivity)
target_include_directories(bench_rwg PRIVATE seq_hdt/include)

add_executable(bench_ram_rwg bench_rwg.cc)
target_compile_definitions(bench_ram_rwg PRIVATE MEM_USE)
target_link_libraries(bench_ram_rwg PRIVATE dycon lib_dynamic_connectivity)
target_include_directories(bench_ram_rwg PRIVATE seq_hdt/include)

add_executable(bench_rmat bench_rmat.cc)
target_link_libraries(bench_rmat PRIVATE dycon lib_dynamic_connectivity)
target_include_directories(bench_rmat PRIVATE seq_hdt/include)


add_subdirectory(bench_distance_query)