/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gopiment <gopiment@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 19:17:06 by gopiment          #+#    #+#             */
/*   Updated: 2026/11/05 19:45:37 by gopiment         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <unistd.h>

// ./codexion
// number_of_coders
// time_to_burnout
// time_to_compile
// time_to_debug
// time_to_refactor
// number_of_compiles_required
// dongle_cooldown
// scheduler

int	main(int argc, char **argv)
{
	t_sim	sim;
	int		i;

	if (!parse_args(argc, argv, &sim.cfg))
		return (1);
	if (!init_sim(&sim))
		return (1);
	i = -1;
	pthread_create(&sim.monitor, NULL, monitor_routine, &sim);
	while (++i < sim.cfg.coders)
		pthread_create(&sim.coder[i].thread, NULL, \
coder_routine, &sim.coder[i]);
	i = -1;
	pthread_join(sim.monitor, NULL);
	while (++i < sim.cfg.coders)
		pthread_join(sim.coder[i].thread, NULL);
	cleanup(&sim);
	return (0);
}
