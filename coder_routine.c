/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_routine.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gopiment <gopiment@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/02 15:32:17 by gopiment          #+#    #+#             */
/*   Updated: 2026/11/05 19:48:54 by gopiment         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	set_order(t_coder *coder, int *first, int *second)
{
	if (coder->left < coder->right)
	{
		*first = coder->left;
		*second = coder->right;
	}
	else
	{
		*first = coder->right;
		*second = coder->left;
	}
}

static void	take_dongles(t_coder *coder, int first, int second)
{
	pthread_mutex_lock(&coder->sim->dongles[first].lock);
	log_state(coder, "has taken a dongle");
	pthread_mutex_lock(&coder->sim->dongles[second].lock);
	log_state(coder, "has taken a dongle");
}

static void	compile_phase(t_coder *coder, int first, int second)
{
	pthread_mutex_lock(&coder->state_lock);
	coder->last_compile_start = now_in_ms() - coder->sim->start_time;
	pthread_mutex_unlock(&coder->state_lock);
	log_state(coder, "is compiling");
	precise_wait(coder->sim->cfg.time_to_compile, coder->sim->start_time);
	pthread_mutex_unlock(&coder->sim->dongles[first].lock);
	pthread_mutex_unlock(&coder->sim->dongles[second].lock);
	pthread_mutex_lock(&coder->state_lock);
	coder->compiles_done++;
	pthread_mutex_unlock(&coder->state_lock);
}

void	*coder_routine(void *data)
{
	t_coder	*coder;
	int		first;
	int		second;

	coder = (t_coder *)data;
	set_order(coder, &first, &second);
	if (coder->left == coder->right)
	{
		pthread_mutex_lock(&coder->sim->dongles[coder->left].lock);
		log_state(coder, "has taken a dongle");
		while (!should_stop(coder->sim))
			usleep(200);
		pthread_mutex_unlock(&coder->sim->dongles[coder->left].lock);
		return (NULL);
	}
	while (!should_stop(coder->sim))
	{
		take_dongles(coder, first, second);
		compile_phase(coder, first, second);
		log_state(coder, "is debugging");
		precise_wait(coder->sim->cfg.time_to_debug, coder->sim->start_time);
		log_state(coder, "is refactoring");
		precise_wait(coder->sim->cfg.time_to_refactor, coder->sim->start_time);
	}
	return (NULL);
}
