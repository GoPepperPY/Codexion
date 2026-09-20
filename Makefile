NAME	= codexion

CC	= cc
CFLAGS	= -Wall -Wextra -Werror -pthread

OBJ_DIR	= objs

SRCS	= main.c parser.c utils_time.c init.c
OBJS	= $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: %.c codexion.h
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re