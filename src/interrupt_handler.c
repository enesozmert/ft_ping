/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   interrupt_handler.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ozmerte <ozmerte@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 20:57:27 by ozmerte          #+#    #+#             */
/*   Updated: 2026/05/02 00:00:00 by ozmerte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <math.h>
#include "header.h"

static double	compute_loss(int sent, int received)
{
	if (sent <= 0)
		return (0.0);
	return (((sent - received) / (double)sent) * 100.0);
}

static double	compute_avg(double sum, int n)
{
	if (n <= 0)
		return (0.0);
	return (sum / (double)n);
}

static double	compute_mdev(const t_ping_result *r)
{
	double	avg;
	double	variance;

	if (r->received_packets <= 0)
		return (0.0);
	avg = r->rtt / (double)r->received_packets;
	variance = (r->rtt_sum_sq / (double)r->received_packets) - (avg * avg);
	if (variance < 0.0)
		variance = 0.0;
	return (sqrt(variance));
}

static void	print_stats(const t_ping *p)
{
	double	loss;
	double	avg;
	double	mdev;
	long	total_ms;

	loss = compute_loss(p->result->sent_packets, p->result->received_packets);
	avg = compute_avg(p->result->rtt, p->result->received_packets);
	mdev = compute_mdev(p->result);
	total_ms = compute_total_ms(&p->time.start_time);
	printf("\n--- %s ping statistics ---\n", p->dest_hostname);
	printf("%d packets transmitted, %d received, %.0f%% packet loss, "
		"time %ldms\n",
		p->result->sent_packets, p->result->received_packets,
		loss, total_ms);
	if (p->result->received_packets > 0)
		printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
			p->result->rtt_min, avg, p->result->rtt_max, mdev);
}

void	interrupt_handler(int sig)
{
	if (sig != SIGINT)
		return ;
	if (g_ping != NULL)
		print_stats(g_ping);
	exit(0);
}
