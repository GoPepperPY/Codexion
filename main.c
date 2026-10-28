/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gopiment <gopiment@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 19:17:06 by gopiment          #+#    #+#             */
/*   Updated: 2026/09/20 04:57:40 by gopiment         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// ./codexion
// number_of_coders
// time_to_burnout
// time_to_compile
// time_to_debug
// time_to_refactor
// number_of_compiles_required
// dongle_cooldown
// scheduler

int   main(int argc, char **argv)
{
	t_sim	sim;
	int		i;

	if (!parse_args(argc, argv, &sim.cfg))
		return (1);
	if (!init_sim(&sim))
		return (1);
	i = -1;
	while (++i < sim.cfg.coders)
		pthread_create(&sim.coder[i].thread, NULL, coder_routine, &sim.coder[i]);
	i = -1;
	// wait for its thread to finish
	while (++i < sim.cfg.coders)
		pthread_join(sim.coder[i].thread, NULL);
	cleanup(&sim);
	return (0);
}
