FROM ubuntu:xenial

RUN apt-get update && apt-get install -y emacs-nox tmux wget curl
RUN apt-get update -y && apt-get install -y make bison flex

RUN echo 'deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-5.0 main' >> /etc/apt/sources.list.d/llvm.list &&\
    echo 'deb-src http://apt.llvm.org/xenial/ llvm-toolchain-xenial-5.0 main' >> /etc/apt/sources.list.d/llvm.list &&\
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -

RUN apt-get update && apt-get install -y clang-5.0 lldb-5.0
RUN apt-get install -y libpcre2-dev inotify-tools

RUN echo "PS1='[coral]\\w \\$ '" >> /etc/bash.bashrc
RUN ln -s /usr/bin/llvm-symbolizer-5.0 /usr/bin/llvm-symbolizer
RUN cp /usr/bin/lldb-5.0 /usr/bin/lldb
RUN cp /usr/bin/lldb-server-5.0 /usr/bin/lldb-server-5.0.1
ENV ASAN_SYMBOLIZER_PATH /usr/bin/llvm-symbolizer

ENV PATH="/work/src/bin:${PATH}"
ADD src /work/src
WORKDIR /work/src
RUN make bin/coral-test bin/coral-jit
CMD coral-jit

