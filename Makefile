NAME	=	ircserv

SRCDIR	=	srcs/
OBJDIR	=	obj/
INCDIR	=	include/

CC			=	clang++
OFLAGS		=	-Wall -Werror -Wextra -std=c++98 -Ofast -MMD -I $(INCDIR)
OFLAGS_DB	=	-Wall -Werror -Wextra -std=c++98 -g -MMD -I $(INCDIR)
BFLAGS		=	-Wall -Werror -Wextra -std=c++98 -Ofast
BFLAGS_DB	=	-Wall -Werror -Wextra -std=c++98 -g

SRC			=	main.cpp \
				utils.cpp \
				Server.cpp \
				User.cpp \
				Command.cpp \
				Sha256.cpp \
				CommandWorkers/userRequest.cpp \
				CommandWorkers/operCmds.cpp \
				CommandWorkers/auth.cpp \
				Channel.cpp \
				CommandWorkers/channelRequest.cpp \
				CommandWorkers/info.cpp

SRCFULL	=	$(addprefix $(SRCDIR), $(SRC))
OBJS	=	$(patsubst $(SRCDIR)%.cpp, $(OBJDIR)%.o, $(SRCFULL))

.PHONY: all debug clean fclean re

all: $(NAME)

$(OBJDIR):
	mkdir -p $(OBJDIR)
	mkdir -p $(OBJDIR)CommandWorkers

-include $(OBJS:.o=.d)
$(OBJDIR)%.o: $(SRCDIR)%.cpp
	$(CC) $(OFLAGS) -c $< -o $@

$(NAME): $(OBJDIR) $(OBJS)
	$(CC) $(BFLAGS) $(OBJS) -o $(NAME)

debug: OFLAGS=$(OFLAGS_DB)
debug: BFLAGS=$(BFLAGS_DB)
debug: $(OBJDIR) $(OBJS)
	$(CC) $(BFLAGS) $(OBJS) -o $(NAME)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all
