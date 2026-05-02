/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ozmerte <ozmerte@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 20:57:27 by ozmerte          #+#    #+#             */
/*   Updated: 2026/05/02 00:00:00 by ozmerte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.h"

t_ping	*g_ping;

static void	free_ping(t_ping *p)
{
	if (p == NULL)
		return ;
	if (p->packet != NULL)
		free(p->packet->packet);
	if (p->payload != NULL)
		free(p->payload->payload);
	free(p->payload);
	free(p->packet);
	free(p->target_addr);
	free(p->result);
	free(p);
}

static int	resolve_and_setup(t_ping *ping, const char *hostname,
		char *ip_buf, size_t ip_buf_len)
{
	if (resolve_hostname(hostname, ip_buf, ip_buf_len) != 0)
		return (1);
	ping->dest_ip_addr = ip_buf;
	ping->dest_hostname = (char *)hostname;
	if (create_socket(ping) != 0)
	{
		fprintf(stderr, "ft_ping: Ping request failed.\n");
		return (1);
	}
	return (0);
}

int	main(int argc, char *argv[])
{
	t_ping	*ping;
	int		verbose_flag;
	char	ip_addr[INET_ADDRSTRLEN];

	ping = NULL;
	g_ping = NULL;
	verbose_flag = 0;
	parse_args(argc, argv, &verbose_flag);
	if (optind >= argc)
	{
		fprintf(stderr, "Usage: %s [-v] [-?] <hostname>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	if (ping_create_allocates(&ping) < 0)
		return (-1);
	ping->verbose_flag = verbose_flag;
	g_ping = ping;
	signal(SIGINT, interrupt_handler);
	if (resolve_and_setup(ping, argv[optind], ip_addr, sizeof(ip_addr)) != 0)
	{
		free_ping(ping);
		return (1);
	}
	free_ping(ping);
	return (0);
}
