/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sim.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gopiment <gopiment@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/02 15:32:32 by gopiment          #+#    #+#             */
/*   Updated: 2026/11/05 19:43:46 by gopiment         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	log_state(t_coder *coder, const char *msg)
{
	long	ts;

	pthread_mutex_lock(&coder->sim->print_lock);
	ts = now_in_ms() - coder->sim->start_time;
	printf("%ld %d %s\n", ts, coder->id, msg);
	pthread_mutex_unlock(&coder->sim->print_lock);
}
