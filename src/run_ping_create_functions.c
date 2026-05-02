/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   run_ping_create_functions.c                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ozmerte <ozmerte@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 20:57:28 by ozmerte          #+#    #+#             */
/*   Updated: 2026/05/01 20:57:28 by ozmerte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.h"

void	run_ping_create_functions(t_ping *ping,
		t_ping_func_entry *funcs, int num_funcs)
{
	int	i;

	i = 0;
	while (i < num_funcs)
	{
		if (funcs[i].func(ping) <= 0)
			exit(EXIT_FAILURE);
		i++;
	}
}
