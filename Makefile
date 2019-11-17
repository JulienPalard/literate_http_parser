NAME	=	parser.so

SRC	=	parser.cpp http_parser.cpp

OBJ	=	$(SRC:.cpp=.o)

CFLAGS	=	-W

LDFLAGS	=	--shared -lrt

$(NAME)	:	$(OBJ)
		g++ $(CFLAGS) $(OBJ) $(LDFLAGS) -o $(NAME)

all	:	$(NAME)

clean	:
		rm -f $(OBJ)

fclean	:	clean
		rm -f $(NAME)

re	:	fclean all

