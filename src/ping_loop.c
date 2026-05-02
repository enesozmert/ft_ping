/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping_loop.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ozmerte <ozmerte@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/02 00:00:00 by ozmerte          #+#    #+#             */
/*   Updated: 2026/05/02 00:00:00 by ozmerte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.h"

void	print_ping_banner(const t_ping *ping)
{
	int	payload_size;
	int	wire_size;

	payload_size = ping->payload->payload_size;
	wire_size = payload_size + (int) sizeof(t_iphdr)
		+ (int) sizeof(t_icmphdr);
	printf("PING %s (%s) %d(%d) bytes of data",
		ping->dest_hostname, ping->dest_ip_addr,
		payload_size, wire_size);
	if (ping->verbose_flag)
		printf(", id 0x%04x = %d", getpid() & 0xFFFF, getpid() & 0xFFFF);
	printf(".\n");
}

static void	refresh_icmp_header(t_ping *ping)
{
	ping->icmp_header->type = ICMP_ECHO;
	ping->icmp_header->code = 0;
	ping->icmp_header->id = htons(getpid() & 0xFFFF);
	ping->icmp_header->sequence = htons(ping->seq);
	ping->icmp_header->checksum = 0;
	ping->icmp_header->checksum = checksum(ping->icmp_header,
			sizeof(t_icmphdr) + ping->payload->payload_size);
}

static int	send_one(t_ping *ping)
{
	refresh_icmp_header(ping);
	if (create_send_request(ping) < 0)
		return (-1);
	if (create_socket_select(ping) < 0)
		return (0);
	create_socket_recvfrom(ping);
	return (1);
}

int	ping_loop(t_ping *ping)
{
	ping->seq = 1;
	ping->result->rtt_min = 0.0;
	ping->result->rtt_max = 0.0;
	ping->result->rtt_sum_sq = 0.0;
	gettimeofday(&ping->time.start_time, NULL);
	while (1)
	{
		FD_ZERO(&ping->read_fds);
		FD_SET(ping->sock_fd, &ping->read_fds);
		send_one(ping);
		ping->seq++;
		sleep(1);
	}
	return (0);
}
