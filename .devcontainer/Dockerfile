FROM ubuntu:jammy

ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && apt-get install -y locales && rm -rf /var/lib/apt/lists/* \
    && localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8
ENV LANG en_US.utf8

RUN apt-get update && apt-get install -y \
    xz-utils \
    build-essential \
    gcovr \
    git \
    tree \
    cmake \
    ruby ruby-dev gem \
    valgrind \
    clang-format \
    clang-tidy \
    doxygen \
    python3 \
    python3-pip

RUN python3 -m pip install lizard jinja2
RUN gem install ceedling
