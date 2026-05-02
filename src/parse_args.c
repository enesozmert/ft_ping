/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_args.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ozmerte <ozmerte@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 20:57:27 by ozmerte          #+#    #+#             */
/*   Updated: 2026/05/01 20:57:27 by ozmerte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "header.h"

static void	print_usage(const char *prog, FILE *stream)
{
	fprintf(stream, "Usage: %s [-v] [-?] <hostname>\n", prog);
}

static void	handle_unknown_opt(const char *prog)
{
	fprintf(stderr, "ft_ping: invalid option -- '%c'\n", optopt);
	print_usage(prog, stderr);
	exit(EXIT_FAILURE);
}

void	parse_args(int argc, char *argv[], int *verbose_flag)
{
	int	opt;

	opterr = 0;
	opt = getopt(argc, argv, "v?");
	while (opt != -1)
	{
		if (opt == 'v')
			*verbose_flag = 1;
		else if (opt == '?' && (optopt == 0 || optopt == '?'))
		{
			print_usage(argv[0], stdout);
			exit(EXIT_SUCCESS);
		}
		else
			handle_unknown_opt(argv[0]);
		opt = getopt(argc, argv, "v?");
	}
}
