/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   create_payload.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ozmerte <ozmerte@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 20:57:27 by ozmerte          #+#    #+#             */
/*   Updated: 2026/05/01 20:57:27 by ozmerte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.h"

int	create_payload(t_ping *ping)
{
	const char	*data;
	uint32_t	length;

	data = "ft_ping-1453-1453-1453-1453-1453-1453-1453-1453--ft_ping";
	length = strlen(data);
	ping->payload->payload = (char *)malloc(length + 1);
	if (ping->payload->payload == NULL)
		return (-1);
	memcpy(ping->payload->payload, data, length + 1);
	ping->payload->payload_size = length;
	return (1);
}
