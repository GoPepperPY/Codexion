/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gopiment <gopiment@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/14 19:47:31 by gopiment          #+#    #+#             */
/*   Updated: 2026/09/16 13:44:20 by gopiment         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

typedef struct s_codex{
	int		coders;
	int		time_to_bo;
	int		time_to_compile;
	int		time_to_debug;
	int		time_to_refactor;
	int 	number_of_compiles_required;
	int 	dongle_cooldown;
	char	*scheduler;
}				t_codex;

# endif
