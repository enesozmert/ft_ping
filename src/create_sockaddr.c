/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   create_sockaddr.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ozmerte <ozmerte@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 20:57:27 by ozmerte          #+#    #+#             */
/*   Updated: 2026/05/02 00:00:00 by ozmerte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.h"

int	create_sockaddr(t_ping *ping)
{
	memset(ping->target_addr, 0, sizeof(*(ping->target_addr)));
	ping->target_addr->sin_family = AF_INET;
	ping->target_addr->sin_port = 0;
	if (inet_pton(AF_INET, ping->dest_ip_addr,
			&ping->target_addr->sin_addr) != 1)
	{
		perror("inet_pton");
		return (-1);
	}
	return (1);
}
