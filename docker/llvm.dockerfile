FROM kubasz51/faasm-faabric-base:0.1.6

# Copy the code in
WORKDIR /code
COPY . .

# Run the main make
RUN make

# Print the clang version
RUN /usr/local/faasm/toolchain/bin/clang --version

# Remove the code
WORKDIR /
RUN rm -r /code

CMD /bin/bash

