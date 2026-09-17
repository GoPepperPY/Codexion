/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gopiment <gopiment@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 21:42:01 by gopiment          #+#    #+#             */
/*   Updated: 2026/09/17 22:00:59 by gopiment         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
 
int	is_positive_integer(const char *str)
{
	int	i;
 
	i = 0;	
	if (!str || !str[0] || strlen(str) > 10)
		return (0);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}
 
int	validate_numbers(char **argv)
{
	int	i;
 
	i = 1;
	while (i <= 7)
	{
		if (!is_positive_integer(argv[i]))
			return (0);
		i++;
	}
	return (1);
}
 
void	assign_numbers(t_codex *cfg, char **argv)
{
	cfg->coders = atoi(argv[1]);
	cfg->time_to_bo = atoi(argv[2]);
	cfg->time_to_compile = atoi(argv[3]);
	cfg->time_to_debug = atoi(argv[4]);
	cfg->time_to_refactor = atoi(argv[5]);
	cfg->number_of_compiles_required = atoi(argv[6]);
	cfg->dongle_cooldown = atoi(argv[7]);
}
 
int	parse_scheduler(char *str, t_scheduler *out)
{
	if (strcmp(str, "fifo") == 0)
		*out = FIFO;
	else if (strcmp(str, "edf") == 0)
		*out = EDF;
	else
		return (0);
	return (1);
}
 
int	parse_args(int argc, char **argv, t_codex *cfg)
{
	if (argc != 9)
		return (fprintf(stderr, "Error: expected 8 arguments\n"), 0);
	if (!validate_numbers(argv))
		return (fprintf(stderr, "Error: args must be positive integers\n"), 0);
	assign_numbers(cfg, argv);
	if (!parse_scheduler(argv[8], &cfg->scheduler))
		return (fprintf(stderr, "Error: scheduler must be fifo or edf\n"), 0);
	if (cfg->coders < 1)
		return (fprintf(stderr, "Error: need at least one coder\n"), 0);
	return (1);
}
