/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   create_socket_recvfrom.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ozmerte <ozmerte@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 20:57:27 by ozmerte          #+#    #+#             */
/*   Updated: 2026/05/02 00:00:00 by ozmerte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.h"

static double	compute_elapsed_ms(t_ping *ping)
{
	double	ms;

	gettimeofday(&ping->time.end_time, NULL);
	ms = (ping->time.end_time.tv_sec - ping->time.start_time.tv_sec)
		* 1000.0;
	ms += (ping->time.end_time.tv_usec - ping->time.start_time.tv_usec)
		/ 1000.0;
	return (ms);
}

static void	update_rtt_stats(t_ping_result *r, double rtt)
{
	if (r->received_packets == 1 || rtt < r->rtt_min)
		r->rtt_min = rtt;
	if (rtt > r->rtt_max)
		r->rtt_max = rtt;
	r->rtt_sum_sq += rtt * rtt;
}

static void	process_echo_reply(t_ping *ping, ssize_t len,
			const struct iphdr *ip_hdr, const struct icmphdr *icmp_hdr)
{
	double	rtt;
	int		icmp_bytes;

	ping->result->received_packets++;
	rtt = compute_elapsed_ms(ping);
	ping->time.elapsed_time = rtt;
	ping->result->rtt += rtt;
	update_rtt_stats(ping->result, rtt);
	icmp_bytes = (int)len - (ip_hdr->ihl * 4);
	printf("%d bytes from %s: icmp_seq=%u ttl=%d time=%.3f ms\n",
		icmp_bytes, ping->dest_ip_addr,
		ntohs(icmp_hdr->un.echo.sequence),
		ip_hdr->ttl, rtt);
}

static void	report_unexpected(const t_ping *ping,
			const struct icmphdr *icmp_hdr)
{
	if (!ping->verbose_flag)
		return ;
	fprintf(stderr,
		"From %s: icmp_seq=%u type=%d code=%d\n",
		ping->dest_ip_addr,
		ntohs(icmp_hdr->un.echo.sequence),
		icmp_hdr->type, icmp_hdr->code);
}

int	create_socket_recvfrom(t_ping *ping)
{
	unsigned char			recv_buf[1024];
	socklen_t				addr_len;
	ssize_t					len;
	const struct iphdr		*ip_hdr;
	const struct icmphdr	*icmp_hdr;

	addr_len = sizeof(*(ping->target_addr));
	len = recvfrom(ping->sock_fd, &recv_buf, sizeof(recv_buf), 0,
			(struct sockaddr *)ping->target_addr, &addr_len);
	if (len <= 0)
		return (-1);
	ip_hdr = (const struct iphdr *)recv_buf;
	icmp_hdr = (const struct icmphdr *)(recv_buf + (ip_hdr->ihl * 4));
	if (icmp_hdr->type == ICMP_ECHOREPLY
		&& ntohs(icmp_hdr->un.echo.id) == (getpid() & 0xFFFF))
		process_echo_reply(ping, len, ip_hdr, icmp_hdr);
	else
		report_unexpected(ping, icmp_hdr);
	return (1);
}
