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

OBJDIR = objs
OBJS = $(SRCS:src/%.cpp=$(OBJDIR)/%.o)



all: $(NAME)

$(NAME): $(OBJS)
	@mkdir -p $(OBJDIR)
	$(CXX) $(INC) $(OBJS) -o $(NAME)

$(OBJDIR)/%.o: src/%.cpp
	$(CXX) $(INC) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
