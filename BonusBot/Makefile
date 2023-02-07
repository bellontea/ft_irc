NAME		= BonusBot

SRCS		= main.cpp Socket.cpp Utils.cpp

OBJS		= ${SRCS:.cpp=.o}

CMPL		= clang++

FLGS		= -Wall -Wextra -Werror -std=c++98

RM			= rm -f

.cpp.o:
			@${CMPL} ${FLGS} -c $< -o ${<:.cpp=.o}

${NAME}:	${OBJS}
			@${CMPL} ${FLGS} -o ${NAME} ${OBJS}
			@echo "\033[1;32mDONE\033[0m"

all:		${NAME}

clean:
			@${RM} ${OBJS}
			@echo "\033[1;32mCLEARED\033[0m"

fclean:		clean
			@${RM} ${NAME}

re:			fclean all

.PHONY:		all clean fclean re
