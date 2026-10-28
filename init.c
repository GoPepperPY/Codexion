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

//Destroys the first count dongle and coder mutexes; used to roll back cleanly if init fails partway.
void	destroy_mutexes(t_sim *sim, int count)
{
	int	i;

	i = -1;
	while (++i < count)
	{
		pthread_mutex_destroy(&sim->dongles[i].lock);
		pthread_mutex_destroy(&sim->coder[i].state_lock);
	}
}

//Allocates the coder and dongle arrays, sets their ids and back-pointers, initializes all mutexes, and records the simulation start time.
int	init_sim(t_sim *sim)
{
	int	counter;

	sim->coder = malloc(sizeof(t_coder) * sim->cfg.coders);
	sim->dongles = malloc(sizeof(t_dongle) * sim->cfg.coders);
	if (!sim->coder || !sim->dongles)
		return (free(sim->dongles), free(sim->coder), 0);
	memset(sim->coder, 0, sizeof(t_coder) * sim->cfg.coders);
	memset(sim->dongles, 0, sizeof(t_dongle) * sim->cfg.coders);
	counter = -1;
	while (++counter < sim->cfg.coders)
	{
		sim->coder[counter].id = counter + 1;
		sim->coder[counter].sim = sim;
		sim->dongles[counter].id = counter + 1;
		sim->dongles[counter].sim = sim;
		sim->coder[counter].left  = counter;
		sim->coder[counter].right = (counter + 1) % sim->cfg.coders;
	}
	if (!init_mutexes(sim))
		return (free(sim->coder), free(sim->dongles), 0);
	sim->stop = 0;
	sim->seq_counter = 0;
	sim->start_time = now_in_ms();
	return (1);
}

//Initializes every dongle lock, coder state lock, and the shared print/sim locks, undoing what was already done if any init fails.
int	init_mutexes(t_sim *sim)
{
	int	counter;

	counter = -1;
	while (++counter < sim->cfg.coders)
	{
		if (pthread_mutex_init(&sim->dongles[counter].lock, NULL) != 0)
			return (destroy_mutexes(sim, counter), 0);
		if (pthread_mutex_init(&sim->coder[counter].state_lock, NULL) != 0)
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

void  cleanup(t_sim *sim)
{
	destroy_mutexes(sim, sim->cfg.coders);
	pthread_mutex_destroy(&sim->print_lock);
	pthread_mutex_destroy(&sim->sim_lock);
	free(sim->coder);
	free(sim->dongles);
}
