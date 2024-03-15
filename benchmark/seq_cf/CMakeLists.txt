add_executable(seq_cf seq_cf.cc)
target_link_libraries(seq_cf PRIVATE dycon)
add_executable(test_CFleaf test_leaf.cc)
target_link_libraries(test_CFleaf PRIVATE dycon)