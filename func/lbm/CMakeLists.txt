set(FAASM_USER lbm)

faasm_func(lbm lbm.c)
target_sources(lbm PRIVATE main.c)
target_include_directories(lbm SYSTEM BEFORE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../ndp")
target_compile_definitions(lbm PUBLIC SPEC SPECCPU NDEBUG NOPERFLIB NOREDUCE SPEC_AUTO_SUPPRESS_OPENMP IN_FAASM)
set(ALL_LBM_FUNCS lbm)

add_custom_target(lbm_all_funcs DEPENDS lbm)
