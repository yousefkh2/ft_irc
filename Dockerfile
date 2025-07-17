FROM debian:latest

RUN apt-get update && apt-get install -y \
    build-essential \
    valgrind \
    cmake \
    irssi \
    net-tools \
    netcat-openbsd \
    telnet \
    python3 \
    python3-pip \
    && apt-get clean

WORKDIR /app

CMD ["bash"]
