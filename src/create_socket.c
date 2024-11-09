#include "header.h"

int create_socket(t_ping *ping)
{
    t_ping_func_entry *functions;

    create_raw_socket(ping);
    printf("enes \n");

    int result6 = get_network_src_mac_addr(ping);
    if (result6 < 0)
    {
        fprintf(stderr, "Source MAC address not found.\n");
        close(ping->sock_fd);
        return 1;
    }

    int result5 = get_network_source_ip_address(ping);
    if (result5 < 0) {
        perror("Failed to get_network_interface_name");
        free(ping);
        return 1;
    }

    int result3 = get_network_interface_name(ping);
    if (result3 < 0) {
        perror("Failed to get_network_interface_name");
        free(ping);
        return 1;
    }

    int result4 = get_network_interface_index(ping);
    if (result4 < 0) {
        perror("Failed to get_network_interface_name");
        free(ping);
        return 1;
    }

    int result1 = get_network_default_gateway(ping);
    if (result1 < 0) {
        perror("Failed to get default gateway");
        free(ping->ifreq);
        close(ping->sock_fd);
        free(ping);
        return 1;
    }

    int result2 = get_network_gateway_mac_address(ping);
    if (result2 == -1) {
        perror("Failed to get_network_gateway_mac_address");
        free(ping->ifreq);
        close(ping->sock_fd);
        free(ping);
        return 1;
    }

    functions = ping_create_functions();
    run_ping_create_functions(ping, functions, 10);

    close(ping->sock_fd);
    return 0;
}
