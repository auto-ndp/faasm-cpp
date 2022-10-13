FROM kubasz51/faasm-faabric-base:0.1.6

# Get the code, build the main targets, and remove the code
ARG SYSROOT_VERSION
RUN mkdir -p /code \
    && git clone -b v${SYSROOT_VERSION} --depth 1 \
        https://github.com/auto-ndp/faasm-cpp \
        /code/cpp \
    && cd /code/cpp \
    && git submodule update --init --depth 1 -f third-party/llvm-project \
    && git submodule update --init --depth 1 -f third-party/wasi-libc \
    && make \
    && /usr/local/faasm/toolchain/bin/clang --version \
    && rm -rf /code
