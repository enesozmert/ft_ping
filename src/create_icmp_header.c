/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   create_icmp_header.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ozmerte <ozmerte@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 20:57:27 by ozmerte          #+#    #+#             */
/*   Updated: 2026/05/02 00:00:00 by ozmerte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.h"

int	create_icmp_header(t_ping *ping)
{
	ping->icmp_header = (struct icmphdr *)ping->packet->packet;
	ping->icmp_header->type = ICMP_ECHO;
	ping->icmp_header->code = 0;
	ping->icmp_header->checksum = 0;
	ping->icmp_header->un.echo.id = htons(getpid() & 0xFFFF);
	ping->icmp_header->un.echo.sequence = htons(0);
	ping->icmp_header->checksum = checksum(ping->icmp_header,
			sizeof(struct icmphdr) + ping->payload->payload_size);
	return (1);
}
