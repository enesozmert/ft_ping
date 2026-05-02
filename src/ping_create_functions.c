/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping_create_functions.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ozmerte <ozmerte@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 20:57:28 by ozmerte          #+#    #+#             */
/*   Updated: 2026/05/02 00:00:00 by ozmerte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.h"

t_ping_func_entry	*ping_create_functions(void)
{
	static t_ping_func_entry	ping_funcs[] = {
	{create_raw_socket, "create_raw_socket"},
	{create_payload, "create_payload"},
	{create_packet, "create_packet"},
	{create_icmp_header, "create_icmp_header"},
	{create_sockaddr, "create_sockaddr"}
	};

	return (ping_funcs);
}
