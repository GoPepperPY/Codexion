/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gopiment <gopiment@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/20 04:22:06 by gopiment          #+#    #+#             */
/*   Updated: 2026/09/20 04:29:14 by gopiment         ###   ########.fr       */
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
	counter = 0;
	while (counter < sim->cfg.coders)
	{
		sim->coders[counter].id = counter + 1;
		sim->coders[counter].sim = sim;
		sim->dongles[counter].id = counter + 1;
		sim->dongles[counter].sim = sim;
		counter++;
	}
	sim->stop = 0;
	sim->seq_counter = 0;
	sim->start_time = now_in_ms();
	return (1);
}
