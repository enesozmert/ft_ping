#include "header.h"

t_ping_func_entry *ping_create_functions(void) {
    static t_ping_func_entry ping_funcs[] = {
        FUNC_ENTRY(create_raw_socket),
        FUNC_ENTRY(create_payload),
        FUNC_ENTRY(create_packet),
        FUNC_ENTRY(create_ethernet_frame),
        FUNC_ENTRY(create_ip_header),
        FUNC_ENTRY(create_icmp_header),
        FUNC_ENTRY(create_sockaddr),
        FUNC_ENTRY(create_send_request),
        FUNC_ENTRY(create_socket_select),
        FUNC_ENTRY(create_socket_recvfrom)
    };
    return ping_funcs;
}