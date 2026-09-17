/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gopiment <gopiment@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 19:47:31 by gopiment          #+#    #+#             */
/*   Updated: 2026/09/17 21:58:22 by gopiment         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <pthread.h>
# include <sys/time.h>

typedef enum e_scheduler
{
	FIFO,
	EDF
}	t_scheduler;

typedef struct s_codex
{
	int			coders;
	long		time_to_bo;
	long		time_to_compile;
	long		time_to_debug;
	long		time_to_refactor;
	int			number_of_compiles_required;
	long		dongle_cooldown;
	t_scheduler	scheduler;
}	t_codex;

/*==========================| PARSER.C |==========================*/
int		is_positive_integer(const char *str);
int		validate_numbers(char **argv);
void	assign_numbers(t_codex *cfg, char **argv);
int		parse_scheduler(char *str, t_scheduler *out);
int		parse_args(int argc, char **, t_codex *cfg);


# endif
