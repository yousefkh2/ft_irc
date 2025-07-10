NAME = ircserv
CXX  = c++ -std=c++17 -Wall -Wextra -Werror
INC  = -I include

SRCS = src/main.cpp \
       src/Server.cpp \
       src/Client.cpp \
       src/Channel.cpp \
       src/CommandHandler.cpp \
       src/Parser.cpp \
       src/Utils.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(INC) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(INC) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
