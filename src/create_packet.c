/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   create_packet.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ozmerte <ozmerte@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 20:57:27 by ozmerte          #+#    #+#             */
/*   Updated: 2026/05/02 00:00:00 by ozmerte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.h"

int	create_packet(t_ping *ping)
{
	ping->packet->packet_len = sizeof(t_icmphdr)
		+ ping->payload->payload_size;
	ping->packet->packet = malloc(ping->packet->packet_len);
	if (!ping->packet->packet)
	{
		perror("malloc");
		return (-1);
	}
	memset(ping->packet->packet, 0, ping->packet->packet_len);
	memcpy(ping->packet->packet + sizeof(t_icmphdr),
		ping->payload->payload, ping->payload->payload_size);
	return (1);
}
