set(FAASM_USER ndp)

function(ndp_func exec_name dir_path)
    faasm_func(${exec_name} ${dir_path})
    target_include_directories(${exec_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/api)
    set(ALL_NDP_FUNCS ${ALL_NDP_FUNCS} ${exec_name} PARENT_SCOPE)
endfunction(ndp_func)

function(ndp_func_c exec_name dir_path)
    faasm_func_c(${exec_name} ${dir_path})
    target_include_directories(${exec_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/api)
    set(ALL_NDP_FUNCS ${ALL_NDP_FUNCS} ${exec_name} PARENT_SCOPE)
endfunction(ndp_func_c)

#ndp_func(argc_argv argc_argv.cpp)
#ndp_func_c(c_example c_example.c)
ndp_func(get get.cpp)
ndp_func(put put.cpp)
ndp_func(thumbnailer_shrink thumbnailer_shrink.cpp)
ndp_func(thumbnailer_decode thumbnailer_decode.cpp)
ndp_func(wordcount wordcount.cpp)
ndp_func(grep grep.cpp)
ndp_func(substr substr.cpp)
ndp_func(sha256sum sha256sum.cpp)
ndp_func(json_parse json_parse.cpp)
ndp_func(pcakmm pcakmm.cpp)
ndp_func(damo damo.cpp)
ndp_func(substrrepl substrrepl.cpp)
ndp_func(damofilter damofilter.cpp)
ndp_func(pcakmmstore pcakmmstore.cpp)
target_link_libraries(pcakmm Eigen34::Eigen)
target_link_libraries(pcakmmstore Eigen34::Eigen)

# 23/09/2020 - removing simd
# WASM-only functions
if (CMAKE_SYSTEM_NAME STREQUAL "Wasm")
    # ndp_func(eigen_vec eigen_vec.cpp)

    # ndp_func(emscripten_check emscripten_check.cpp)
    # target_link_libraries(emscripten_check emscripten)

    # TODO - fix BLAS check function
    # include_directories(/usr/local/faasm/llvm-sysroot/include/clapack)
    # ndp_func_c(blas_check blas_check.c)
    # target_link_libraries(blas_check ${FAASM_BLAS_LIBS})
endif()

# faasm_dynamic_func(dynlink dynlink.cpp)
# set(ALL_NDP_FUNCS ${ALL_NDP_FUNCS} dynlink)

# Custom target to group all the ndp functions
add_custom_target(ndp_all_funcs DEPENDS ${ALL_NDP_FUNCS})

# Turn on WASM profiling
#add_definitions(-DWASM_PROF=1)
#ndp_func(float_perf float_perf.cpp)

