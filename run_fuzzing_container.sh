#!/bin/sh
docker run -it --tmpfs /home/user/.local/share/rs-flip:noexec,size=100m,uid=1000,gid=1000 --tmpfs /home/user/workspace/build/out:noexec,size=1000m,uid=1000,gid=1000 flip:latest /bin/bash
