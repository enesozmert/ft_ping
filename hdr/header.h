#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

typedef struct s_ping_result
{
    int sent_packets;
    int received_packets;
    double rtt;
} t_ping_result;

#define ARP_REQUEST 1
#define ARP_REPLY 2

struct arp_header {
    unsigned short hardware_type;
    unsigned short protocol_type;
    unsigned char hardware_len;
    unsigned char protocol_len;
    unsigned short opcode;
    unsigned char sender_mac[6];
    unsigned char sender_ip[4];
    unsigned char target_mac[6];
    unsigned char target_ip[4];
};

int resolve_hostname(const char *hostname, char *ip_str, size_t ip_str_len);


int create_socket(const char *ip_addr, t_ping_result *result);
unsigned short checksum(void *buffer, int length);
int get_network_interface_index(int sockfd);
void parse_args(int argc, char *argv[], int *verbose_flag);

int get_interface_ip_by_id(unsigned int interface_id, char *ip_address, size_t size);
uint32_t get_source_ip_address();
unsigned char *find_src_mac_addr(int sockfd);
