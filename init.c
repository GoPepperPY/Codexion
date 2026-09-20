/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gopiment <gopiment@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/20 04:22:06 by gopiment          #+#    #+#             */
/*   Updated: 2026/09/20 05:30:18 by gopiment         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	init_sim(t_sim *sim)
{
	int	counter;

	sim->coders = malloc(sizeof(t_coder) * sim->cfg.coders);
	sim->dongles = malloc(sizeof(t_dongle) * sim->cfg.coders);
	if (!sim->coders || !sim->dongles)
	{
		free(sim->coders);
		free(sim->dongles);
		return (0);
	}
	memset(sim->coders, 0, sizeof(t_coder) * sim->cfg.coders);
	memset(sim->dongles, 0, sizeof(t_dongle) * sim->cfg.coders);
	counter = -1;
	while (++counter < sim->cfg.coders)
	{
		sim->coders[counter].id = counter + 1;
		sim->coders[counter].sim = sim;
		sim->dongles[counter].id = counter + 1;
		sim->dongles[counter].sim = sim;
	}
    init_mutexes(sim);
	sim->stop = 0;
	sim->seq_counter = 0;
	sim->start_time = now_in_ms();
	return (1);
}

void	init_mutexes(t_sim *sim)
{
	int	counter;

	counter = -1;
	while (++counter < sim->cfg.coders)
	{
		pthread_mutex_init(&sim->dongles[counter].lock, NULL);
		pthread_mutex_init(&sim->coders[counter].state_lock, NULL);
	}
	pthread_mutex_init(&sim->print_lock, NULL);
	pthread_mutex_init(&sim->sim_lock, NULL);
}
