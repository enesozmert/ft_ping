/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   create_socket_select.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ozmerte <ozmerte@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 20:57:27 by ozmerte          #+#    #+#             */
/*   Updated: 2026/05/01 20:57:27 by ozmerte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.h"

int	create_socket_select(t_ping *ping)
{
	int	ret;

	ping->time.timeout.tv_sec = 1;
	ping->time.timeout.tv_usec = 0;
	ret = select(ping->sock_fd + 1, &ping->read_fds, NULL, NULL,
			&ping->time.timeout);
	if (ret <= 0)
		return (-1);
	return (1);
}
