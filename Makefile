NAME = ircserv
CXX  = c++ -std=c++17 -Wall -Wextra -Werror
INC  = -I include

SRCS = src/main.cpp \
	src/Server.cpp \
	src/Client.cpp \
	src/Channel.cpp \
	src/CommandHandler.cpp \
	src/AuthHandlers.cpp \
	src/ChannelHandlers.cpp \
	src/Parser.cpp \
	src/Utils.cpp \
	src/Helpers.cpp \
	src/Mode.cpp

OBJDIR = objs
OBJS = $(SRCS:src/%.cpp=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(INC) $(OBJS) -o $(NAME)

$(OBJDIR)/%.o: src/%.cpp | $(OBJDIR)
	$(CXX) $(INC) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -f $(OBJS)
      
fclean: clean
	rm -f $(NAME)
	rm -rf $(OBJDIR)

re: fclean all
