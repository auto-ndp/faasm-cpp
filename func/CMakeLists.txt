cmake_minimum_required(VERSION 3.8.0)
project(faasm-func)

# -----------------------------------------
# Most of the variables used transitively used here like compiler or linker
# flags are set in the sysroot file (WasiToolchain.cmake) and are defined in
# the build environment file (faasmtools/build.py)
# -----------------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FAASM_NATIVE_INSTALL $ENV{FAASM_NATIVE_INSTALL_DIR})

if (CMAKE_SYSTEM_NAME STREQUAL "Wasm")
    # For Faasm functions, we add `_faasm_zygote` as an exported symbol
    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} -Xlinker --export=_faasm_zygote"
    )
    message(STATUS "Detected wasm build (sysroot=$ENV{FAASM_WASM_SYSROOT})")
    set(CMAKE_EXECUTABLE_SUFFIX ".wasm")
endif ()


if (CMAKE_SYSTEM_NAME STREQUAL "Wasm")
    # ----------------------
    # Wasm build
    # ----------------------

    set(FAASM_FUNC_LIBS faasm c-printscan-long-double)

    function(faasm_func_c func_name file_name)
        add_executable(${func_name} ${file_name})
        target_link_libraries(${func_name} ${FAASM_FUNC_LIBS})
    endfunction(faasm_func_c)

    function(faasm_func func_name file_name)
        add_executable(${func_name} ${file_name})
        target_link_libraries(${func_name} ${FAASM_FUNC_LIBS})
    endfunction(faasm_func)

    function(faasm_dynamic_func func_name file_name)
        faasm_func(${func_name} ${file_name})

        set_target_properties(${func_name} PROPERTIES
            LINK_FLAGS $ENV{FAASM_WASM_EXE_LINKER_FLAGS_SHARED}
        )

    endfunction(faasm_dynamic_func)
else ()
    # ----------------------
    # Native build
    # ----------------------

    function(faasm_func_c func_name file_name)
        add_executable(${func_name} ${file_name})

        target_link_directories(${func_name} PRIVATE "${FAASM_NATIVE_INSTALL}/lib")
        target_include_directories(${func_name} PRIVATE "${FAASM_NATIVE_INSTALL}/include")

        target_link_libraries(${func_name} libfaasm.a)
    endfunction(faasm_func_c)

    function(faasm_func func_name file_name)
        faasm_func_c(${func_name} ${file_name})
    endfunction(faasm_func)

    function(faasm_dynamic_func func_name file_name)
        faasm_func(${func_name} ${file_name})
        target_link_libraries(${func_name} util)
    endfunction(faasm_dynamic_func)
endif ()

add_library(Eigen34 INTERFACE)
target_compile_features(Eigen34 INTERFACE cxx_std_17)
target_include_directories(Eigen34 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/eigen-3.4)
target_compile_definitions(Eigen34 INTERFACE EIGEN_INITIALIZE_MATRICES_BY_ZERO=1
    EIGEN_NO_AUTOMATIC_RESIZING=1 EIGEN_NO_IO=1 EIGEN_DONT_PARALLELIZE=1 EIGEN_NO_CUDA=1)
add_library(Eigen34::Eigen ALIAS Eigen34)

add_subdirectory(errors)
# add_subdirectory(lbm)
add_subdirectory(darknet)
add_subdirectory(demo)
add_subdirectory(dynlink)
add_subdirectory(mpi)
add_subdirectory(ndp)
add_subdirectory(omp)
# add_subdirectory(povray)

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/nab")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/nab)
endif ()
