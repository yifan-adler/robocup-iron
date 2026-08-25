FROM --platform=linux/amd64 ubuntu:18.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    || (sed -i -e 's|archive.ubuntu.com/ubuntu|old-releases.ubuntu.com/ubuntu|g' \
                  -e 's|security.ubuntu.com/ubuntu|old-releases.ubuntu.com/ubuntu|g' \
                  /etc/apt/sources.list && apt-get update)

RUN apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        coreutils \
        libboost-all-dev \
        procps \
        python3 \
        unzip \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

ENV HOME=/tmp

CMD ["bash"]
