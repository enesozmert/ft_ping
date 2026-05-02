/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   create_raw_socket.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ozmerte <ozmerte@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 20:57:27 by ozmerte          #+#    #+#             */
/*   Updated: 2026/05/02 00:00:00 by ozmerte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.h"

int	create_raw_socket(t_ping *ping)
{
	int	sock_fd;
	int	ttl;

	sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock_fd < 0)
	{
		fprintf(stderr, "ft_ping: socket: %s\n", strerror(errno));
		return (-1);
	}
	ttl = 64;
	if (setsockopt(sock_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
	{
		fprintf(stderr, "ft_ping: setsockopt: %s\n", strerror(errno));
		close(sock_fd);
		return (-1);
	}
	ping->sock_fd = sock_fd;
	return (1);
}
