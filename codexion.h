/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gopiment <gopiment@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 19:47:31 by gopiment          #+#    #+#             */
/*   Updated: 2026/09/20 05:46:02 by gopiment         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include "pthread.h"
# include "stdio.h"
# include "stdlib.h"
# include "string.h"
# include "unistd.h"
# include "sys/time.h"

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

struct s_sim;

typedef struct s_dongle
{
	int				id;
	pthread_mutex_t	lock;
	pthread_cond_t	cond;
	int				available;
	long			available_at;
	struct s_sim	*sim;
}	t_dongle;

typedef struct s_coder
{
	int				id;
	pthread_t		thread;
	long			last_compile_start;
	long			compiles_done;
	pthread_mutex_t	state_lock;
	int				left;
	int				right;
	struct s_sim	*sim;
}	t_coder;

typedef struct s_sim
{
	t_codex			cfg;
	long			start_time;
	t_dongle		*dongles;
	t_coder			*coders;
	pthread_mutex_t	print_lock;
	pthread_mutex_t	sim_lock;
	int				stop;
	long			seq_counter;
	pthread_t		monitor;
}	t_sim;

/*==========================| PARSER.C |==========================*/
int		is_positive_integer(const char *str);
int		validate_numbers(char **argv);
void	assign_numbers(t_codex *cfg, char **argv);
int		parse_scheduler(char *str, t_scheduler *out);
int		parse_args(int argc, char **, t_codex *cfg);

/*==========================| UTILS_TIME.C |==========================*/
long	now_in_ms(void);
void	precise_wait(long duration, long start_time);


/*==========================| INIT.C |==========================*/
int		init_sim(t_sim *sim);
int		init_mutexes(t_sim *sim);
void	destroy_mutexes(t_sim *sim, int count);

# endif
