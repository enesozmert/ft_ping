#include "header.h"

ping_create_func_t *ping_create_functions(void) {
    static ping_create_func_t ping_funcs[] = {
        create_raw_socket,
        create_packet,
        create_ethernet_frame,
        create_payload,
        create_ip_header,
        create_icmp_header,
        create_sockaddr,
        create_send_request
    };
    return ping_funcs;
}