/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gopiment <gopiment@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/17 22:30:27 by gopiment          #+#    #+#             */
/*   Updated: 2026/09/17 22:30:43 by gopiment         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	now_in_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((long)tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void	precise_wait(long duration, long start_time)
{
	long	begin;

	begin = now_in_ms() - start_time;
	while ((now_in_ms() - start_time) - begin < duration)
		usleep(200);
}
