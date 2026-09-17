# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:     #
#                                                     +:+ +:+         +:+       #
#    By: gopiment <gopiment@student.42porto.com>    +#+  +:+       +#+          #
#                                                 +#+#+#+#+#+   +#+             #
#    Created: 2026/09/17 20:40:00 by gopiment          #+#    #+#              #
#    Updated: 2026/09/17 20:40:00 by gopiment         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	= codexion

CC		= cc
CFLAGS	= -Wall -Wextra -Werror -pthread

SRCS	= main.c parser.c utils_time.c
OBJS	= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c codexion.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re