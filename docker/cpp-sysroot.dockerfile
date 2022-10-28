FROM kubasz51/faasm-llvm:10.0.1 as llvm

FROM kubasz51/faasm-faabric-base:0.4.0
ARG SYSROOT_VERSION
SHELL ["/bin/bash", "-c"]
ENV CPP_DOCKER="on"

# Copy the toolchain in from the LLVM container
COPY --from=llvm /usr/local/faasm /usr/local/faasm

RUN apt-get update \
    && apt-get upgrade --yes --no-install-recommends \
    && apt-get install --yes --no-install-recommends autotools-dev libltdl-dev \
    && apt-get clean autoclean --yes \
    && apt-get autoremove --yes

# Get the code and submodules
ARG SYSROOT_VERSION
RUN mkdir -p /code \
    && git clone -b v${SYSROOT_VERSION} \
        https://github.com/auto-ndp/faasm-cpp \
        /code/cpp \
    && cd /code/cpp \
    # Update submodules (not LLVM)
    && git submodule update --init --depth 1 -f third-party/faabric \
    && git submodule update --init --depth 1 -f third-party/faasm-clapack \
    && git submodule update --init --depth 1 -f third-party/libffi \
    && git submodule update --init --depth 1 -f third-party/wasi-libc \
    && git submodule update --init --depth 1 -f third-party/FFmpeg \
    && git submodule update --init --depth 1 -f third-party/zlib \
    && git submodule update --init --depth 1 -f third-party/libpng \
    && git submodule update --init --depth 1 -f third-party/ImageMagick \
    && git submodule update --init --depth 1 -f third-party/tensorflow

# Install the faasmtools Python lib
RUN cd /code/cpp \
    && ./bin/create_venv.sh \
    && source venv/bin/activate \
    && pip3 install -r requirements.txt \
    && pip3 install .

# Build all the targets
RUN cd /code/cpp \
    && source venv/bin/activate \
    # Build native Faasm libraries (static and shared)
    && inv \
        libfaasm --native \
        libfaasmp --native \
        libfaasmpi --native \
        libfaasm --native --shared \
        libfaasmp --native --shared \
        libfaasmpi --native --shared \
    # Install toolchain files
    && inv install \
    # Build ported third-pary WASM libraries (libc first as it is needed in the
    # others)
    && inv \
        libc \
        clapack \
        clapack --clean --shared \
    #   ffmpeg \
        libffi \
        # To build imagemagick, we need to build zlib and libpng
        zlib \
        libpng \
        imagemagick \
    #    tensorflow \
    # Build Faasm WASM libraries
    && inv \
        libemscripten \
        libfaasm \
        libfaasmp \
        libfaasmpi

# CLI setup
WORKDIR /code/cpp
ENV TERM xterm-256color

RUN echo ". /code/cpp/bin/workon.sh" >> ~/.bashrc
CMD ["/bin/bash", "-l"]
