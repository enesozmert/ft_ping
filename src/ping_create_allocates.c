/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping_create_allocates.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ozmerte <ozmerte@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 20:57:27 by ozmerte          #+#    #+#             */
/*   Updated: 2026/05/02 00:00:00 by ozmerte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.h"

static int	alloc_one(void **dst, size_t size)
{
	*dst = malloc(size);
	if (!*dst)
		return (-1);
	memset(*dst, 0, size);
	return (0);
}

static void	free_ping_chain(t_ping *p)
{
	if (!p)
		return ;
	free(p->payload);
	free(p->packet);
	free(p->target_addr);
	free(p->result);
	free(p);
}

int	ping_create_allocates(t_ping **ping)
{
	*ping = malloc(sizeof(t_ping));
	if (!*ping)
		return (-1);
	memset(*ping, 0, sizeof(t_ping));
	if (alloc_one((void **)&(*ping)->result, sizeof(t_ping_result)) < 0
		|| alloc_one((void **)&(*ping)->target_addr,
			sizeof(struct sockaddr_in)) < 0
		|| alloc_one((void **)&(*ping)->packet, sizeof(t_packet)) < 0
		|| alloc_one((void **)&(*ping)->payload, sizeof(t_payload)) < 0)
	{
		free_ping_chain(*ping);
		*ping = NULL;
		return (-1);
	}
	return (1);
}
