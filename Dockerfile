# docker build -t ft_irc .
# docker run -it -p 6667:6667 -v "$(pwd):/app" ft_irc
# ./ircserv 6667 password
# docker exec -it *container_id or name* irssi -n yourname
# /connect -! -4 -noautosendcmd localhost 6667 1234

# optional: -nocap when connecting and remove CAP

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
