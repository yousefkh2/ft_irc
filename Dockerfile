# docker build -t ft_irc .
# docker run --rm --name ft_irc -it 6667:6667 -v "$(pwd):/app" ft_irc
# ./ircserv 6667 password
# docker exec -it ft_irc bash
# /connect -! -4 -noautosendcmd localhost 6667 1234

# optional: -nocap when connecting and remove CAP

# ──────────────────────────────────────────────────────────────────────────────
# Build with:   docker build -t ft_irc .
# Run   with:   docker run --name ft_irc -it -p 6667:6667 -v "$(pwd):/app" ft_irc
# Inside the container:
#     ./ircserv 6667 pass
#
# In a second terminal:
#     docker exec -it ft_irc \
#       weechat -q \
#       -r "/server add local 127.0.0.1/6667 -password=pass" \
#       -r "/connect local"
# ──────────────────────────────────────────────────────────────────────────────

FROM debian:bookworm-slim

# Silence interactive prompts (tzdata, etc.)
ENV DEBIAN_FRONTEND=noninteractive

# System deps + WeeChat (console client) + common debug tools
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
        build-essential  \
        valgrind         \
        cmake            \
        weechat          \
        netcat-openbsd   \
        telnet           \
        net-tools        \
        python3          \
        python3-pip      \
		expect			\
        ca-certificates  \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

# Your project lives here
WORKDIR /app

# Make port 6667 visible on `docker run -p`
EXPOSE 6667

CMD ["bash"]
