# docker build -t docker-irssi -f Dockerfile .
# docker run -it --rm -v $(pwd):/app docker-irssi

FROM debian:latest

RUN apt-get update && apt-get install -y \
    build-essential \
    valgrind \
    cmake \
	irssi \
    && apt-get clean

WORKDIR /app

CMD ["bash"]
