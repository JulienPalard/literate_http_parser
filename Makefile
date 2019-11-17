NAME	=	parser

SRC	=	parser.c http_parser.c

OBJ	=	$(SRC:.c=.o)

CFLAGS	=	-W

LDFLAGS	=	-lrt

$(NAME)	:	$(OBJ)
		cc $(CFLAGS) $(OBJ) $(LDFLAGS) -o $(NAME)

all	:	$(NAME)

clean	:
		rm -f $(OBJ)

fclean	:	clean
		rm -f $(NAME)

re	:	fclean all
