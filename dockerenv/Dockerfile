FROM ubuntu:xenial

RUN apt-get update && apt-get install -y emacs-nox tmux wget curl

RUN echo 'deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-5.0 main' >> /etc/apt/sources.list.d/llvm.list &&\
    echo 'deb-src http://apt.llvm.org/xenial/ llvm-toolchain-xenial-5.0 main' >> /etc/apt/sources.list.d/llvm.list &&\
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -

RUN apt-get update && apt-get install -y --no-install-recommends \
    libllvm-5.0 clang-5.0

RUN apt-get install -y make bison flex
RUN apt-get install -y gdb
RUN echo "PS1='[coral]\\w \\$ '" >> /etc/bash.bashrc
RUN ln -s /usr/bin/llvm-symbolizer-5.0 /usr/bin/llvm-symbolizer
ENV ASAN_SYMBOLIZER_PATH /usr/bin/llvm-symbolizer
WORKDIR /work
