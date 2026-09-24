/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gopiment <gopiment@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/20 04:22:06 by gopiment          #+#    #+#             */
/*   Updated: 2026/09/22 18:26:36 by gopiment         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	destroy_mutexes(t_sim *sim, int count)
{
	int	i;

	i = -1;
	while (++i < count)
	{
		pthread_mutex_destroy(&sim->dongles[i].lock);
		pthread_mutex_destroy(&sim->coders[i].state_lock);
	}
}

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
    if (!init_mutexes(sim))
	{
		free(sim->coders);
		free(sim->dongles);
		return (0);
	}
	sim->stop = 0;
	sim->seq_counter = 0;
	sim->start_time = now_in_ms();
	return (1);
}

int	init_mutexes(t_sim *sim)
{
	int	counter;

	counter = -1;
	while (++counter < sim->cfg.coders)
	{
		if (pthread_mutex_init(&sim->dongles[counter].lock, NULL) != 0)
			return (destroy_mutexes(sim, counter), 0);
		if (pthread_mutex_init(&sim->coders[counter].state_lock, NULL) != 0)
		{
			pthread_mutex_destroy(&sim->dongles[counter].lock);
			return (destroy_mutexes(sim, counter), 0);
		}
	}
	if (pthread_mutex_init(&sim->print_lock, NULL) != 0)
		return (destroy_mutexes(sim, sim->cfg.coders), 0);
	if (pthread_mutex_init(&sim->sim_lock, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->print_lock);
		return (destroy_mutexes(sim, sim->cfg.coders), 0);
	}
	return (1);
}
