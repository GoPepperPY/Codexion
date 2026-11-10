/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gopiment <gopiment@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/09 14:49:35 by gopiment          #+#    #+#             */
/*   Updated: 2026/11/09 14:49:40 by gopiment         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	heap_create(t_heap *heap, int capacity, t_scheduler scheduler)
{
	heap->data = malloc(sizeof(t_request) * capacity);
	if (!heap->data)
		return (0);
	heap->size = 0;
	heap->capacity = capacity;
	heap->scheduler = scheduler;
	return (1);
}

void	heap_free(t_heap *heap)
{
	free(heap->data);
	heap->data = NULL;
	heap->size = 0;
}

t_request	*heap_peek(t_heap *heap)
{
	if (heap->size == 0)
		return (NULL);
	return (&heap->data[0]);
}
