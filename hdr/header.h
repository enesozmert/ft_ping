#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdint.h>
#include <sys/time.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/if_arp.h>
#include <sys/select.h>
#include <netdb.h>

typedef struct s_ethernet_frame
{
    unsigned char dest_mac[6];
    unsigned char src_mac[6];
    unsigned short eth_type;
} t_ethernet_frame;

// ICMP Reply Struct
typedef struct s_icmp_reply
{
    struct iphdr ip_hdr;
    struct icmphdr icmp_hdr;
    unsigned char payload[64];
} t_icmp_reply;

typedef struct s_payload{
    char *payload;
    int payload_size;
} t_payload;

typedef struct s_packet{
    unsigned char *packet;
    int icmp_packet_size;
    int packet_len;
} t_packet;

typedef struct s_time{
    struct timeval start_time;
    struct timeval end_time;
    struct timeval timeout;
    double elapsed_time;
} t_time;

typedef struct s_ping_result
{
    int sent_packets;
    int received_packets;
    double rtt;
} t_ping_result;

typedef struct s_ping{
    int sock_fd;
    fd_set read_fds;
    char *dest_ip_addr;
    char *dest_hostname;
    uint32_t src_ip_addr;
    struct iphdr *ip_header;
    struct icmphdr *icmp_header;
    char *network_interface_name;
    unsigned char *src_mac;
    unsigned char *dest_mac;
    unsigned char *gateway_mac;
    char *gateway_ip;
    struct sockaddr_ll *target_addr;
    struct ifreq *ifreq;
    t_ethernet_frame *ethernet_frame;
    t_icmp_reply icmp_reply;
    t_payload *payload;
    t_packet packet;
    t_time time;
    t_ping_result *result;
} t_ping;

typedef int (*ping_create_func_t)(t_ping *);

int resolve_hostname(const char *hostname, char *ip_str, size_t ip_str_len);

int create_raw_socket(t_ping *ping);
int create_packet(t_ping *ping);
int create_payload(t_ping *ping);
int create_socket(t_ping *ping);

int create_icmp_header(t_ping *ping);
int create_ip_header(t_ping *ping);

int create_ethernet_frame(t_ping *ping);

int create_sockaddr(t_ping *ping);

int create_send_request(t_ping *ping);

ping_create_func_t *ping_create_functions();
void run_ping_create_functions(t_ping *ping, ping_create_func_t funcs[], int num_funcs);

unsigned short checksum(void *buffer, int length);
void parse_args(int argc, char *argv[], int *verbose_flag);

uint32_t get_source_ip_address();
unsigned char *find_src_mac_addr(int sockfd);

int get_network_interface_index(t_ping *ping);
int get_network_interface_name(t_ping *ping);
int get_network_gateway_mac_address(t_ping *ping);
int get_network_default_gateway(t_ping *ping);
