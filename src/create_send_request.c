/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   create_send_request.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ozmerte <ozmerte@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 20:57:27 by ozmerte          #+#    #+#             */
/*   Updated: 2026/05/02 00:00:00 by ozmerte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.h"

int	create_send_request(t_ping *ping)
{
	ping->result->sent_packets++;
	gettimeofday(&ping->time.start_time, NULL);
	if (sendto(ping->sock_fd, ping->packet->packet,
			ping->packet->packet_len, 0,
			(struct sockaddr *)ping->target_addr,
			sizeof(*ping->target_addr)) < 0)
	{
		if (ping->verbose_flag)
			perror("sendto");
		return (-1);
	}
	FD_ZERO(&ping->read_fds);
	FD_SET(ping->sock_fd, &ping->read_fds);
	return (1);
}
