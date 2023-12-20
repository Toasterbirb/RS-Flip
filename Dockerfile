FROM archlinux

# Update everything
RUN pacman -Syu --noconfirm

# Install a C compiler and other build dependencies
RUN pacman -S --noconfirm base-devel cmake git doctest nlohmann-json

# Setup a new user
ARG USER=user
RUN useradd -m ${USER} && chown -R ${USER}:${USER} /home/${USER}

# Setup a working directory
WORKDIR /home/${USER}
COPY --chown=${USER} . workspace
USER ${USER}

# Clean out old build files etc.
RUN rm -rf workspace/build .local/share/rs-flip

RUN mkdir -p workspace/build && cd workspace/build && cmake .. && make -j$(nproc)

ENTRYPOINT [ "/bin/sh" ]
