# docker build -t docker-irssi -f Dockerfile .
# docker run -it --rm -v $(pwd):/app docker-irssi

FROM debian:latest

RUN apt-get update && apt-get install -y \
    # just to build & check for leaks in the same container
    # better to remove before eval to make container lightweight
    build-essential \
    valgrind \
    cmake \
    # --------
	irssi \
	net-tools \
	netcat-openbsd \
    && apt-get clean

WORKDIR /app

CMD ["bash"]
