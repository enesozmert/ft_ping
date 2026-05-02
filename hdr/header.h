/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   header.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ozmerte <ozmerte@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 20:57:28 by ozmerte          #+#    #+#             */
/*   Updated: 2026/05/02 00:00:00 by ozmerte         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADER_H
# define HEADER_H

# ifndef _DEFAULT_SOURCE
#  define _DEFAULT_SOURCE
# endif
# ifndef _GNU_SOURCE
#  define _GNU_SOURCE
# endif
# ifndef _DARWIN_C_SOURCE
#  define _DARWIN_C_SOURCE
# endif

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <signal.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <errno.h>
# include <stdint.h>
# include <sys/time.h>
# include <netdb.h>
# include <sys/select.h>

/*
 * POSIX-portable wire-format structs (Linux iphdr/icmphdr & BSD ip/icmp
 * agnostic). Layout matches RFC 791 (IP) and RFC 792 (ICMP) on the wire,
 * so both Linux raw socket reads and macOS/BSD reads parse correctly.
 */
# ifndef IPPROTO_ICMP
#  define IPPROTO_ICMP 1
# endif
# ifndef ICMP_ECHO
#  define ICMP_ECHO 8
# endif
# ifndef ICMP_ECHOREPLY
#  define ICMP_ECHOREPLY 0
# endif

typedef struct s_iphdr
{
	uint8_t		vhl;
	uint8_t		tos;
	uint16_t	tot_len;
	uint16_t	id;
	uint16_t	frag_off;
	uint8_t		ttl;
	uint8_t		protocol;
	uint16_t	check;
	uint32_t	saddr;
	uint32_t	daddr;
}	t_iphdr;

typedef struct s_icmphdr
{
	uint8_t		type;
	uint8_t		code;
	uint16_t	checksum;
	uint16_t	id;
	uint16_t	sequence;
}	t_icmphdr;

typedef struct s_payload
{
	char	*payload;
	int		payload_size;
}	t_payload;

typedef struct s_packet
{
	unsigned char	*packet;
	int				packet_len;
}	t_packet;

typedef struct s_time
{
	struct timeval	start_time;
	struct timeval	end_time;
	struct timeval	timeout;
	double			elapsed_time;
}	t_time;

typedef struct s_ping_result
{
	int		sent_packets;
	int		received_packets;
	double	rtt;
	double	rtt_min;
	double	rtt_max;
	double	rtt_sum_sq;
}	t_ping_result;

typedef struct s_ping
{
	int					sock_fd;
	fd_set				read_fds;
	char				*dest_ip_addr;
	char				*dest_hostname;
	t_icmphdr			*icmp_header;
	struct sockaddr_in	*target_addr;
	t_payload			*payload;
	t_packet			*packet;
	t_time				time;
	t_ping_result		*result;
	uint16_t			seq;
	int					verbose_flag;
}	t_ping;

extern t_ping		*g_ping;

typedef int			(*t_ping_create_func)(t_ping *);

typedef struct s_ping_func_entry
{
	t_ping_create_func	func;
	const char			*func_name;
}	t_ping_func_entry;

int					ping_create_allocates(t_ping **ping);
int					resolve_hostname(const char *hostname, char *ip_str,
						size_t ip_str_len);

int					create_raw_socket(t_ping *ping);
int					create_packet(t_ping *ping);
int					create_payload(t_ping *ping);
int					create_socket(t_ping *ping);

int					create_icmp_header(t_ping *ping);
int					create_sockaddr(t_ping *ping);
int					create_send_request(t_ping *ping);
int					create_socket_select(t_ping *ping);
int					create_socket_recvfrom(t_ping *ping);
int					ping_loop(t_ping *ping);
void				print_ping_banner(const t_ping *ping);

t_ping_func_entry	*ping_create_functions(void);
void				run_ping_create_functions(t_ping *ping,
						t_ping_func_entry *funcs, int num_funcs);

unsigned short		checksum(void *buffer, int length);
void				parse_args(int argc, char *argv[], int *verbose_flag);
double				calculate_rtt(const struct timeval *start,
						const struct timeval *end);
long				compute_total_ms(const struct timeval *start);
double				compute_elapsed_ms(const struct timeval *start,
						struct timeval *end);

void				interrupt_handler(int sig);

#endif
