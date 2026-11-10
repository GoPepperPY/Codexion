/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gopiment <gopiment@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/09 14:50:12 by gopiment          #+#    #+#             */
/*   Updated: 2026/11/10 12:06:26 by gopiment         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	swap(t_request *ticket1, t_request *ticket2)
{
	t_request	tmp;

	tmp = *ticket1;
	*ticket1 = *ticket2;
	*ticket2 = tmp;
}

int	higher_priority(t_heap *heap, t_request *ticket1, t_request *ticket2)
{
	if (heap->scheduler == EDF)
	{
		if (ticket1->deadline != ticket2->deadline)
			return (ticket1->deadline < ticket2->deadline);
		if (ticket1->seq != ticket2->seq)
			return (ticket1->seq < ticket2->seq);
		return (ticket1->coder_id < ticket2->coder_id);
	}
	return (ticket1->seq < ticket2->seq);
}

static int	better(t_heap *heap, int ticket1, int ticket2)
{
	if (higher_priority(heap, &heap->data[ticket1], &heap->data[ticket2]))
		return (ticket1);
	return (ticket2);
}

void	heap_push(t_heap *heap, t_request req)
{
	int	counter;
	int	parent;

	heap->data[heap->size] = req;
	counter = heap->size;
	heap->size++;
	while (counter > 0)
	{
		parent = (counter - 1) / 2;
		if (!higher_priority(heap, &heap->data[counter], &heap->data[parent]))
			break ;
		counter = parent;
		swap(&heap->data[counter], &heap->data[parent]);
	}
}

void	heap_pop(t_heap *heap)
{
	int	counter;
	int	best;

	if (heap->size == 0)
		return ;
	heap->size--;
	heap->data[0] = heap->data[heap->size];
	counter = 0;
	while (1)
	{
		best = counter;
		if (2 * counter + 1 < heap->size)
			best = better(heap, best, 2 * counter + 1);
		if (2 * counter + 2 < heap->size)
			best = better(heap, best, 2 * counter + 2);
		if (best == counter)
			break ;
		swap(&heap->data[counter], &heap->data[best]);
		counter = best;
	}
}
