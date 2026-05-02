/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   resolve_hostname.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ozmerte <ozmerte@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 20:57:28 by ozmerte          #+#    #+#             */
/*   Updated: 2026/05/01 20:57:28 by ozmerte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.h"

int	resolve_hostname(const char *hostname, char *ip_str, size_t ip_str_len)
{
	struct addrinfo		hints;
	struct addrinfo		*res;
	struct sockaddr_in	*addr;
	int					result;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	result = getaddrinfo(hostname, NULL, &hints, &res);
	if (result != 0)
	{
		fprintf(stderr, "ping: %s: %s\n", hostname, gai_strerror(result));
		return (1);
	}
	addr = (struct sockaddr_in *)res->ai_addr;
	if (inet_ntop(AF_INET, &addr->sin_addr, ip_str, ip_str_len) == NULL)
	{
		perror("inet_ntop");
		freeaddrinfo(res);
		return (1);
	}
	freeaddrinfo(res);
	return (0);
}
