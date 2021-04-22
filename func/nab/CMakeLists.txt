set(FAASM_USER nab)

faasm_func(nab src/nabmd.c)
target_sources(nab PRIVATE
    src/sff.c src/nblist.c src/prm.c src/memutil.c src/molio.c src/molutil.c src/errormsg.c
    src/binpos.c src/rand2.c src/select_atoms.c src/reslib.c src/database.c src/traceback.c src/chirvol.c
    src/specrand/specrand.c
    src/regex-alpha/regcomp.c src/regex-alpha/regerror.c src/regex-alpha/regexec.c src/regex-alpha/regfree.c)
target_include_directories(nab SYSTEM BEFORE PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/specrand"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/regex-alpha" "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}/../ndp")
target_compile_definitions(nab PUBLIC SPEC SPECCPU NDEBUG NOPERFLIB NOREDUCE SPEC_AUTO_SUPPRESS_OPENMP IN_FAASM)
set(ALL_NAB_FUNCS nab)

add_custom_target(nab_all_funcs DEPENDS nab)
