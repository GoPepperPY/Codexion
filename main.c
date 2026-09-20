/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gopiment <gopiment@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 19:17:06 by gopiment          #+#    #+#             */
/*   Updated: 2026/09/20 03:03:30 by gopiment         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int argc, char **argv)
{
	t_sim	sim;

	if (!parse_args(argc, argv, &sim.cfg))
		return (1);
	if (!init_sim(&sim))
		return (1);
	return (0);
}
