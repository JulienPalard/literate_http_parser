NAME	=	parser.so

SRC	=	parser.c http_parser.c

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
