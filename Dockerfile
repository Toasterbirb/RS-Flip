FROM archlinux

# Update everything
RUN pacman -Syu --noconfirm

# Install a C compiler and other build dependencies
RUN pacman -S --noconfirm base-devel cmake git doctest nlohmann-json bash afl++ vim

# Setup a new user
ARG USER=user
RUN useradd -m ${USER} && mkdir -p /home/${USER}/.local/share/rs-flip && chown -R ${USER}:${USER} /home/${USER}

# Setup a working directory
WORKDIR /home/${USER}
COPY --chown=${USER} . workspace
USER ${USER}

# Clean out old build files etc.
RUN rm -rf workspace/build .local/share/rs-flip/*

RUN mkdir -p /home/${USER}/workspace/build/out && cd workspace/build && cmake -DCMAKE_CXX_COMPILER=afl-c++ -DFUZZ=ON .. && make -j$(nproc)

# Copy the fuzzing test cases into the build directory
RUN cp -r workspace/fuzzing_testcases workspace/build/in
